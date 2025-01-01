#include "mainwindow.h"
#include "gotodialog.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), fractalsActionGroup(this) {
  ui->setupUi(this);
  connect(ui->centralwidget, &ViewerWidget::startCalculating, this,
          &MainWindow::startCalculating);
  connect(ui->centralwidget, &ViewerWidget::completed, this,
          &MainWindow::completed);
  connect(ui->actionCopy, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::copyCoords);
  connect(ui->actionGoTo, &QAction::triggered, this,
          &MainWindow::openGoToDialog);

  connect(ui->actionIncrease_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::increaseIterations);
  connect(ui->actionDecrease_iterations, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::decreaseIterations);

  ui->actionCopy->setShortcut(QKeySequence::Copy);
  ui->actionQuit->setShortcut(QKeySequence::Quit);

  connect(ui->actionRandomize_palette, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::randomizePalette);
  connect(ui->actionHome, &QAction::triggered, ui->centralwidget,
          &ViewerWidget::resetCurrentFractal);

  QIcon icon(":/new/prefix1/icon.ico");
  QApplication::setWindowIcon(icon);

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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeFractal(ChangeFractalAction *src,
                               const fractals::PointwiseFractal &fractal) {
  ui->centralwidget->changeFractal(fractal);
}

void MainWindow::startCalculating(double d, int iterations) {
  std::stringstream ss;
  ss << "Calculating radius " << std::setprecision(2) << d << " to "
     << iterations << " iterations";

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::completed(double d, int min_depth, int max_depth, double avg,
                           double skipped, double time) {
  std::stringstream ss;
  ss << "Radius " << std::setprecision(2) << d << " completed in " << time
     << " seconds, depth " << min_depth << "-" << max_depth;

  // Log the number of iterations skipped if you want
  // (also, could display this somehow)
  // std::cout << "Average " << avg << " iterations\n";
  // std::cout << "Skipped " << skipped << " iterations\n";

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::openGoToDialog() {
  GoToDialog dialog;
  QString x, y, r, i;
  ui->centralwidget->getCoords(x, y, r, i);
  dialog.setCoords(x, y, r, i);
  if (dialog.exec()) {
    dialog.getCoords(x, y, r, i);
    ui->centralwidget->setCoords(x, y, r, i);
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
