#include "ViewerWidget.h"

#include <QApplication>
#include <QImage>
#include <QMoveEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>

// Only for debugging
#include <iostream>

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent},
      mandelbrot{fractals::make_mandelbrot(fractals::Algorithm::Default)} {}

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  assert(image.width() > 0);

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();

  mandelbrot->calculate_async(viewport);
}

void ViewerWidget::draw() {
  QPainter painter(this);

  painter.drawImage(this->rect(), image);
}

void ViewerWidget::resizeEvent(QResizeEvent *event) {

  // auto old_width = viewport.width;
  // auto old_height = viewport.height;

  image = QImage(event->size(), QImage::Format_RGB32);

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();
  mandelbrot->set_aspect_ratio(viewport); // Tweak width and height

  calculate();
}

void ViewerWidget::wheelEvent(QWheelEvent *event) {
  double r = 1.0 - event->pixelDelta().y() / 100.0;
  if (r > 2.0)
    r = 2.0;
  if (r < 0.5)
    r = 0.5;
  mandelbrot->zoom(r, event->position().x(), event->position().y(), viewport);
  zoomChanged(mandelbrot->width());
  calculate();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    mandelbrot->scroll(press_x - event->pos().x(), press_y - event->pos().y(),
                       viewport);
    calculate();
    press_x = event->pos().x();
    press_y = event->pos().y();
  }
}

void ViewerWidget::mousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    press_x = event->pos().x();
    press_y = event->pos().y();
  }
}

void ViewerWidget::MyViewport::region_updated(int x, int y, int w, int h) {
  // Note will be called on different threads
  widget->update();
}

void ViewerWidget::MyViewport::finished(double width, int min_depth,
                                        int max_depth, double render_time) {
  widget->completed(width, min_depth, max_depth, render_time);
}

void ViewerWidget::increaseIterations() {
  mandelbrot->increase_iterations(viewport);
  calculate();
}
