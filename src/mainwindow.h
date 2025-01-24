#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fractal.hpp"
#include <QAction>
#include <QActionGroup>
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
    void completed(double zoom, int min_depth, int max_depth, double, double,
                   double time);
    void openGoToDialog();
    void changeFractal(class ChangeFractalAction *src,
                       const fractals::PointwiseFractal &fractal);
    void fractalChanged(const std::string &name);
    void cancelAnimations();

  private:
    Ui::MainWindow *ui;
    QActionGroup fractalsActionGroup;
    QActionGroup zoomSpeedActionGroup;
    QActionGroup threadingActionGroup;
};

class ChangeFractalAction : public QAction {
  Q_OBJECT
public:
  ChangeFractalAction(const char *name,
                      const fractals::PointwiseFractal &fractal, bool checked);

signals:
  void changeFractal(ChangeFractalAction *src,
                     const fractals::PointwiseFractal &);

private slots:
  void select(bool checked);

private:
  const fractals::PointwiseFractal &fractal;
};

#endif // MAINWINDOW_H
