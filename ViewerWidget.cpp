#include "ViewerWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QMimeData>
#include <QMoveEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>

// Only for debugging
#include "ViewCoords.hpp"
#include <iomanip>
#include <iostream>

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, colourMap{fractals::make_colourmap()},
      mandelbrot{fractals::make_mandelbrot()} {
  connect(&timer, &QTimer::timeout, this, &ViewerWidget::timer2);
}

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  assert(image.width() > 0);

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();

  mandelbrot->calculate_async(viewport, *colourMap);
}

void ViewerWidget::draw() {
  pending_redraw = 0;
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
  move_x = event->pos().x();
  move_y = event->pos().y();
}

void ViewerWidget::mousePressEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    press_x = event->pos().x();
    press_y = event->pos().y();
  }
}

void ViewerWidget::MyViewport::region_updated(int x, int y, int w, int h) {
  // Note will be called on different threads
  if (!widget->pending_redraw) {
    ++widget->pending_redraw;
    widget->update();
  }
}

void ViewerWidget::MyViewport::finished(double width, int min_depth,
                                        int max_depth, double render_time) {
  widget->completed(width, min_depth, max_depth, render_time);
}

void ViewerWidget::increaseIterations() {
  mandelbrot->increase_iterations(viewport);
  calculate();
}

void ViewerWidget::decreaseIterations() {
  mandelbrot->decrease_iterations(viewport);
  calculate();
}

void ViewerWidget::toggleAutoMode() {

  if (timer.isActive())
    timer.stop();
  else
    timer.start(100);
}

void ViewerWidget::timer2() {
  // std::cout << "Timer called!\n";
  mandelbrot->zoom(0.99, move_x, move_y, viewport);
  zoomChanged(mandelbrot->width());
  calculate();
}

void ViewerWidget::timerEvent(QTimerEvent *evt) {}

void ViewerWidget::copyCoords()
{
    auto c = mandelbrot->get_coords();

    auto cx = c.x;
    auto cy = c.y;

    int zeros = fractals::count_zeros(c.r);
    int width = 4 + zeros * 0.30103;

    std::stringstream ss;
    ss << std::setprecision(width) << cx << std::endl;
    ss << cy << std::endl;
    ss << c.r << std::endl;

    QClipboard *clipboard = QApplication::clipboard();

    QMimeData *data = new QMimeData;
    data->setImageData(image);
    data->setText(ss.str().c_str());
    clipboard->setMimeData(data);
}

void ViewerWidget::getCoords(QString &x, QString &y, QString &r,
                             QString &i) const {
  auto c = mandelbrot->get_coords();

  int zeros = fractals::count_zeros(c.r);
  int width = 3 + zeros * 0.30103;

  {
    std::stringstream ss;
    ss << std::setprecision(width) << c.x << std::endl;
    x = ss.str().c_str();
  }

  {
    std::stringstream ss;
    ss << std::setprecision(width) << c.y << std::endl;
    y = ss.str().c_str();
  }

  {
    std::stringstream ss;
    ss << std::setprecision(width) << c.r << std::endl;
    r = ss.str().c_str();
  }

  {
    std::stringstream ss;
    ss << c.max_iterations << std::endl;
    i = ss.str().c_str();
  }
}

bool ViewerWidget::setCoords(const QString &x, const QString &y,
                             const QString &r, const QString &i) {
  fractals::ViewCoords coords;

  fractals::ViewCoords::value_type cx;
  fractals::ViewCoords::value_type cy;
  fractals::ViewCoords::value_type cr;

  {
    std::istringstream ss(x.toStdString());
    ss >> coords.x;
  }

  {
    std::istringstream ss(y.toStdString());
    ss >> coords.y;
  }
  {
    std::istringstream ss(r.toStdString());
    ss >> coords.r;
  }

  {
    std::istringstream ss(i.toStdString());
    ss >> coords.max_iterations;
  }

  mandelbrot->set_coords(coords, viewport);
  calculate();
  return true;
}
