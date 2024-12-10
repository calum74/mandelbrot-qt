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

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Q) {
    QApplication::instance()->exit(0);
  }
}
