#include "ViewerWidget.h"

#include <QImage>
#include <QMoveEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>

// Only for debugging
#include <iostream>

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, mandelbrot{fractals::make_mandelbrot(
                           fractals::Algorithm::CoherentRegions)} {}

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
  image = QImage(event->size(), QImage::Format_RGB32);
  calculate();
}

void ViewerWidget::wheelEvent(QWheelEvent *event) {
  auto dy = event->pixelDelta().y();
  double r = 1.0 - event->pixelDelta().y() / 100.0;
  if (r > 2.0)
    r = 2.0;
  if (r < 0.5)
    r = 0.5;
  mandelbrot->zoom(r, event->position().x(), event->position().y(),
                   image.width(), image.height());
  calculate();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    mandelbrot->scroll(press_x - event->pos().x(), press_y - event->pos().y(),
                       image.width(), image.height());
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
  // !! Can specify which region to repaint
  widget->repaint();
}