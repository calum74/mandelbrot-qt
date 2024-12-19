#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fractal.hpp"
#include <QEvent>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void startCalculating(double d, int i);
    void completed(double zoom, int min_depth, int max_depth, double time);
    void openGoToDialog();

  private:
    Ui::MainWindow *ui;
};

// TODO: Move this elsewhere
class ChangeFractalAction : public QAction {
  Q_OBJECT
public:
  ChangeFractalAction(const char *name,
                      const fractals::PointwiseFractal &fractal)
      : QAction{name}, fractal{fractal} {

    connect(this, &QAction::triggered, this, &ChangeFractalAction::select);
  }

signals:
  void changeFractal(const fractals::PointwiseFractal &);

private slots:
  void select() { changeFractal(fractal); }

private:
  const fractals::PointwiseFractal &fractal;
};

#endif // MAINWINDOW_H
