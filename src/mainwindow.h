#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "RenderingMetrics.hpp"
#include "fractal.hpp"
#include "view_parameters.hpp"
#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QMainWindow>
#include <nlohmann/json_fwd.hpp>

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
    void completed(const fractals::RenderingMetrics *metrics);
    void openGoToDialog();
    void changeFractal(class ChangeFractalAction *src,
                       const fractals::PointwiseFractal &fractal);
    void fractalChanged(const char *name);
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

class Bookmark : public QAction {
  Q_OBJECT
public:
  Bookmark(const nlohmann::json &json);

signals:
  void select(const fractals::view_parameters *);

private:
  fractals::view_parameters params;
};

#endif // MAINWINDOW_H
