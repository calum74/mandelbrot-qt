#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include "fractal.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image;

  struct MyViewport : public fractals::Viewport {
    ViewerWidget *widget;
    void region_updated(int x, int y, int w, int h) override;
    void finished(double width, int min_depth, int max_depth,
                  double render_time) override;
  } viewport;

  std::unique_ptr<fractals::ColourMap> colourMap;
  std::unique_ptr<fractals::Fractal> mandelbrot;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y;

  void calculate();
  void draw();

  std::atomic<int> pending_redraw;

  QTimer timer;

public:
  explicit ViewerWidget(QWidget *parent = nullptr);

  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

  void increaseIterations();
  void decreaseIterations();
  void toggleAutoMode();

private slots:
  void timer2();

signals:
  void zoomChanged(double newZoom);
  void completed(double width, int min_depth, int max_depth, double time);
};

#endif // VIEWERWIDGET_H
