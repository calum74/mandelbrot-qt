#include "mainwindow.h"
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

    ui->actionCopy->setShortcut(QKeySequence::Copy);
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

void MainWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Q:
    QApplication::instance()->exit(0);
    break;
  case Qt::Key_I:
    ui->centralwidget->increaseIterations();
    break;
  case Qt::Key_O:
    ui->centralwidget->decreaseIterations();
    break;
  case Qt::Key_A:
    ui->centralwidget->toggleAutoMode();
    break;
  }

  QMainWindow::keyPressEvent(event);
}
