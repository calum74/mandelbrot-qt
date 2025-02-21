#include "mainwindow.h"
#include "addbookmark.h"
#include "gotodialog.h"
#include "json.hpp"
#include "ui_mainwindow.h"
#include "view_coords.hpp"
#include <QFile>
#include <QKeyEvent>
#include <cmath>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

MainWindow::MainWindow(const std::shared_ptr<SharedBookmarks> &bookmarks0,
                       QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), bookmarks(bookmarks0),
      fractalsActionGroup(this), zoomSpeedActionGroup(this),
      threadingActionGroup(this) {
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
  connect(ui->actionAutomatic_gradient, &QAction::triggered, ui->centralwidget,
    &ViewerWidget::enableAutoGradient);
  
  connect(ui->actionAuto_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::enableAutoDepth);
  connect(ui->actionQuick_save, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::quickSave);
  connect(ui->actionScale_palette, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::scalePalette);
  connect(ui->actionOpen, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::open);
  connect(ui->actionNew_window, &QAction::triggered, this,
          &MainWindow::newWindow);
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
  connect(ui->actionAdd_bookmark, &QAction::triggered, this,
          &MainWindow::addBookmark);
  connect(ui->actionShow_bookmark_locations, &QAction::triggered, this,
          &MainWindow::showBookmarks);

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

  if (!bookmarks) {
    bookmarks = std::make_shared<SharedBookmarks>();
    loadBookmarks(QFile(":/new/prefix1/bookmarks.json"), false, true);
    loadBookmarks(getBookmarksFile(), true, false);
  } else {
    loadBookmarks(QFile(":/new/prefix1/bookmarks.json"), false, false);
    for (auto &bm : bookmarks->bookmarks)
      doAddBookmark(bm, false, false);
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
                               const fractals::fractal &fractal) {
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
#ifndef NDEBUG
  ss << "DEBUG BUILD ";
#endif
  ss << "Radius " << std::setprecision(2);
  fractals::log_radius(ss, metrics->log_radius);
  ss << " completed in " << std::fixed << metrics->render_time_seconds << " seconds, depth "
     << (int)metrics->min_depth << "-" << (int)metrics->max_depth;

  // Extra metrics
  if(ui->actionDeveloper_mode->isChecked()) {
    ss << " (skipped " << (100.0 * metrics->average_skipped_iterations / metrics->average_iterations) << "%)";
  }

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
    const char *name, const fractals::fractal &fractal, bool checked)
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

Bookmark::Bookmark(const fractals::view_parameters &params) : params(params) {
  connect(this, &QAction::triggered, this, &Bookmark::triggered);
  auto last = params.title.rfind('/');

  setText(params.title.c_str() + (last >= 0 ? last + 1 : 0));
}

void Bookmark::triggered(bool checked) { selected(&params); }

void MainWindow::doAddBookmark(const fractals::view_parameters &params,
                               bool isUser, bool isBuiltin) {
  // Split the name of the parameter
  std::string_view name = params.title;
  QMenu *menu = ui->menuBookmarks_2;

  for (int p = name.find('/'); p >= 0; p = name.find('/')) {
    QString first = std::string(name.substr(0, p)).c_str();
    name = name.substr(p + 1);
    QMenu *sub = 0;
    for (auto *action : menu->actions()) {
      if (action->menu() && action->text() == first) {
        sub = action->menu();
        break;
      }
    }
    if (!sub) {
      sub = new QMenu(first, menu);
      menu->addMenu(sub);
    }
    menu = sub;
  }

  auto *bookmark = new Bookmark(params);
  connect(bookmark, &Bookmark::selected, ui->centralwidget,
          &ViewerWidget::openBookmark);
  menu->addAction(bookmark);
  if (isUser)
    bookmarks->bookmarks.push_back(params);
  if (isBuiltin)
    bookmarks->builtinBookmarks.push_back(params);
}

void MainWindow::addBookmark() {
  AddBookmark dialog;
  if (dialog.exec()) {
    fractals::view_parameters params;
    ui->centralwidget->getCoords(params);
    params.title = dialog.getName().toStdString();
    doAddBookmark(params, true, false);
    saveBookmarks();
  }
}

void MainWindow::loadBookmarks(QFile &&file, bool isUser, bool isBuiltin) {
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray contents = file.readAll();
    nlohmann::json data =
        nlohmann::json::parse(contents.begin(), contents.end());

    // Turn it into JSON
    for (auto &item : data) {
      auto params = read_json(item);
      doAddBookmark(params, isUser, isBuiltin);
    }
  }
}

QFile MainWindow::getBookmarksFile() { return QFile("bookmarks.json"); }

void MainWindow::saveBookmarks() {
  QFile file = getBookmarksFile();
  if (file.open(QIODevice::WriteOnly)) {
    nlohmann::json js = nlohmann::json::array();
    for (const auto &bookmark : bookmarks->bookmarks) {
      js.push_back(write_json(bookmark));
    }
    auto contents = js.dump(4);
    file.write(contents.data(), contents.size());
  }
}

void MainWindow::newWindow() {
  auto w = new MainWindow(bookmarks);
  w->show();
}

void MainWindow::closeEvent(QCloseEvent *) {
  delete ui->centralwidget;
  ui->centralwidget = 0;
}

void MainWindow::showBookmarks(bool checked) {
  if (checked) {
    std::vector<fractals::view_parameters> bookmarksToShow =
        bookmarks->builtinBookmarks;
    bookmarksToShow.insert(bookmarksToShow.end(), bookmarks->bookmarks.begin(),
                           bookmarks->bookmarks.end());
    ui->centralwidget->showBookmarks(bookmarksToShow.data(),
                                     bookmarksToShow.size());
  } else {
    ui->centralwidget->hideBookmarks();
  }
}