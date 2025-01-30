#include "mainwindow.h"
#include "gotodialog.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QKeyEvent>
#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), fractalsActionGroup(this),
      zoomSpeedActionGroup(this), threadingActionGroup(this) {
  ui->setupUi(this);
  connect(ui->centralwidget, &ViewerWidget::startCalculating, this,
          &MainWindow::startCalculating);
  connect(ui->centralwidget, &ViewerWidget::completed, this,
          &MainWindow::completed);
  connect(ui->actionCopy, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::copyCoords);
  connect(ui->actionPaste_coords, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::pasteCoords);
  connect(ui->actionGoTo, &QAction::triggered, this,
          &MainWindow::openGoToDialog);

  connect(ui->actionIncrease_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::increaseIterations);
  connect(ui->actionDecrease_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::decreaseIterations);

  ui->actionCopy->setShortcut(QKeySequence::Copy);
  ui->actionQuit->setShortcut(QKeySequence::Quit);

  connect(ui->actionRandomize_palette, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::recolourPalette);
  connect(ui->actionHome, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::resetCurrentFractal);

  connect(ui->actionMultithreading, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::enableThreading);
  connect(ui->actionMax_threads, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::maxThreading);
  connect(ui->actionSingle_threaded, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::singleThreaded);

  connect(ui->actionOversampling, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::enableOversampling);

  connect(ui->actionAuto_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::enableAutoDepth);
  connect(ui->actionQuick_save, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::quickSave);
  connect(ui->actionScale_palette, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::scalePalette);
  connect(ui->actionOpen, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::open);
  connect(ui->actionSave, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::save);
  connect(ui->actionZoom_in, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::smoothZoomIn);
  connect(ui->actionZoom_out, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::zoomOut);
  connect(ui->actionAutozoom, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::autoZoom);
  connect(ui->actionZoom_continuously, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::zoomAtCursor);

  connect(ui->actionAnimate_to_here, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::animateToHere);
  connect(ui->actionQuality_animation, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::setQualityAnimation);
  connect(ui->actionFast_animation, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::setFastAnimation);
  connect(ui->actionVery_fast_animation, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::setFastestAnimation);
  connect(ui->centralwidget, &ViewerWidget::fractalChanged, this,
          &MainWindow::fractalChanged);

  zoomSpeedActionGroup.setExclusionPolicy(
      QActionGroup::ExclusionPolicy::Exclusive);
  zoomSpeedActionGroup.addAction(ui->actionQuality_animation);
  zoomSpeedActionGroup.addAction(ui->actionFast_animation);
  zoomSpeedActionGroup.addAction(ui->actionVery_fast_animation);

  threadingActionGroup.setExclusionPolicy(
      QActionGroup::ExclusionPolicy::Exclusive);
  threadingActionGroup.addAction(ui->actionMultithreading);
  threadingActionGroup.addAction(ui->actionMax_threads);
  threadingActionGroup.addAction(ui->actionSingle_threaded);

  QIcon icon(":/new/prefix1/icon.ico");
  QApplication::setWindowIcon(icon);

  QFile file(":/new/prefix1/bookmarks.json");
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray contents = file.readAll();
    nlohmann::json data =
        nlohmann::json::parse(contents.begin(), contents.end());

    // Turn it into JSON
    for (auto &item : data) {
      auto *bookmark = new Bookmark(item);
      connect(bookmark, &Bookmark::selected, ui->centralwidget,
              &ViewerWidget::openBookmark);
      ui->menuBookmarks_2->addAction(bookmark);
    }
  }

  fractalsActionGroup.setExclusionPolicy(
      QActionGroup::ExclusionPolicy::Exclusive);

  // Dynamically populate the fractals menu
  bool first = true;
  for (auto &[name, f] : ui->centralwidget->listFractals()) {
    auto *action = new ChangeFractalAction(name.c_str(), f, first);
    first = false;
    connect(action, &ChangeFractalAction::changeFractal, this,
            &MainWindow::changeFractal);
    ui->menuFractal->addAction(action);
    fractalsActionGroup.addAction(action);
  }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::changeFractal(ChangeFractalAction *src,
                               const fractals::PointwiseFractal &fractal) {
  ui->centralwidget->changeFractal(fractal);
}

void MainWindow::startCalculating(double d, int iterations) {
  std::stringstream ss;
  ss << "Calculating radius " << std::setprecision(2);
  fractals::log_radius(ss, d);
  ss << " to " << iterations << " iterations";

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::completed(const fractals::RenderingMetrics *metrics) {
  std::stringstream ss;
  ss << "Radius " << std::setprecision(2);
  fractals::log_radius(ss, metrics->log_radius);
  ss << " completed in " << metrics->render_time_seconds << " seconds, depth "
     << (int)metrics->min_depth << "-" << (int)metrics->max_depth;

  // Log the number of iterations skipped if you want
  // (also, could display this somehow)
  // std::cout << "Average " << avg << " iterations\n";
  // std::cout << "Skipped " << skipped << " iterations\n";

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::openGoToDialog() {
  GoToDialog dialog;
  fractals::view_parameters params;
  ui->centralwidget->getCoords(params);
  dialog.setCoords(params);
  if (dialog.exec()) {
    dialog.getCoords(params);
    ui->centralwidget->setCoords(params);
  }
}

ChangeFractalAction::ChangeFractalAction(
    const char *name, const fractals::PointwiseFractal &fractal, bool checked)
    : QAction{name}, fractal{fractal} {
  setCheckable(true);
  setChecked(checked);

  connect(this, &QAction::toggled, this, &ChangeFractalAction::select);
}

void ChangeFractalAction::select(bool checked) {
  if (checked) {
    changeFractal(this, fractal);
  }
}

void MainWindow::cancelAnimations() {}

void MainWindow::fractalChanged(const char *name) {
  for (auto *action : fractalsActionGroup.actions()) {
    action->setChecked(action->text() == name);
  }
}

void parse(const nlohmann::json &js, fractals::view_coords::value_type &v) {
  std::stringstream ss(js.get<std::string>());
  ss >> v;
}

Bookmark::Bookmark(const nlohmann::json &js) {

  connect(this, &QAction::triggered, this, &Bookmark::triggered);

  if (js.contains("Name")) {
    auto name = js["Name"];
    if (name.is_string()) {
      setText(name.get<std::string>().c_str());
    }
  }
  if (js.contains("Re"))
    parse(js["Re"], params.coords.x);
  if (js.contains("Im"))
    parse(js["Im"], params.coords.y);
  if (js.contains("Radius"))
    parse(js["Radius"], params.coords.r);
  if (js.contains("Iterations"))
    params.coords.max_iterations = js["Iterations"].get<int>();
  if (js.contains("Gradient"))
    params.colour_gradient = js["Gradient"].get<double>();
  if (js.contains("Colour"))
    params.colour_seed = js["Colour"].get<int>();
}

void Bookmark::triggered(bool checked) { selected(&params); }