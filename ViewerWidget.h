#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QWidget>

#include "fractal.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image;
  fractals::Viewport viewport;
  std::unique_ptr<fractals::Fractal> mandelbrot;

  int press_x, press_y;

  void calculate();
  void draw();

public:
  explicit ViewerWidget(QWidget *parent = nullptr);

  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

signals:
};

#endif // VIEWERWIDGET_H
