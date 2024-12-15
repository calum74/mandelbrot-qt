#include "mainwindow.h"
#include "gotodialog.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->centralwidget, &ViewerWidget::zoomChanged, this,
            &MainWindow::zoomChanged);
    connect(ui->centralwidget, &ViewerWidget::completed, this,
            &MainWindow::completed);
    connect(ui->actionCopy, &QAction::triggered, ui->centralwidget,
            &ViewerWidget::copyCoords);
    connect(ui->actionGoTo, &QAction::triggered, this,
            &MainWindow::openGoToDialog);

    connect(ui->actionIncrease_iterations, &QAction::triggered, ui->centralwidget, &ViewerWidget::increaseIterations);
    connect(ui->actionDecrease_iterations, &QAction::triggered, ui->centralwidget, &ViewerWidget::decreaseIterations);

    ui->actionCopy->setShortcut(QKeySequence::Copy);
    ui->actionQuit->setShortcut(QKeySequence::Quit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::zoomChanged(double d) {
  std::stringstream ss;
  ss << "Width = " << d;

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::completed(double d, int min_depth, int max_depth,
                           double time) {
  std::stringstream ss;
  ss << "Width = " << d << " completed in " << time << " seconds, depth "
     << min_depth << "-" << max_depth;

  ui->statusbar->showMessage(ss.str().c_str());
}

void MainWindow::openGoToDialog() {
  GoToDialog dialog;
  // TODO: Populate current coords
  QString x, y, r, i;
  ui->centralwidget->getCoords(x, y, r, i);
  dialog.setCoords(x, y, r, i);
  if (dialog.exec()) {
    dialog.getCoords(x, y, r, i);
    ui->centralwidget->setCoords(x, y, r, i);
  }
}
