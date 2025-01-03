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

#include "mandelbrot.hpp"
#include "registry.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>

void register_fractals(fractals::Registry &r);

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, colourMap{fractals::make_colourmap()},
      renderer{fractals::make_renderer()} {}

ViewerWidget::~ViewerWidget() { renderer.reset(); }

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  startCalculating(renderer->width(), renderer->iterations());

  assert(image.width() > 0);

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();

  renderer->calculate_async(viewport, *colourMap);
}

void ViewerWidget::draw() {
  pending_redraw = 0;
  QPainter painter(this);

  painter.drawImage(this->rect(), image);
}

void ViewerWidget::resizeEvent(QResizeEvent *event) {

  renderer->set_aspect_ratio(event->size().width(), event->size().height());

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();
  image = QImage(event->size(), QImage::Format_RGB32);
  viewport.invalidateAllPixels();
  calculate();
}

void ViewerWidget::wheelEvent(QWheelEvent *event) {
  double r = 1.0 - event->angleDelta().y() / 128.0;
  if (r > 2.0)
    r = 2.0;
  if (r < 0.5)
    r = 0.5;
  if (r != 1.0) {
    renderer->zoom(r, event->position().x(), event->position().y(), viewport);
    calculate();
  }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    renderer->scroll(press_x - event->pos().x(), press_y - event->pos().y(),
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
                                        int max_depth, double avg,
                                        double skipped, double render_time) {
  widget->completed(width, min_depth, max_depth, avg, skipped, render_time);
}

void ViewerWidget::MyViewport::discovered_depth(int points,
                                                double discovered_depth) {
  if (widget->renderer)
    widget->renderer->discovered_depth(points, discovered_depth);
}

void ViewerWidget::increaseIterations() {
  renderer->increase_iterations(viewport);
  calculate();
}

void ViewerWidget::decreaseIterations() {
  renderer->decrease_iterations(viewport);
  calculate();
}

void ViewerWidget::copyCoords()
{
  auto c = renderer->get_coords();

  auto cx = c.x;
  auto cy = c.y;

  int zeros = fractals::count_fractional_zeros(c.r);
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
  auto c = renderer->get_coords();

  int zeros = fractals::count_fractional_zeros(c.r);
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
  fractals::view_coords coords;

  fractals::view_coords::value_type cx;
  fractals::view_coords::value_type cy;
  fractals::view_coords::value_type cr;

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

  renderer->set_coords(coords, viewport);
  calculate();
  return true;
}

void ViewerWidget::recolourPalette() {
  colourMap->randomize();
  renderer->redraw(viewport);
  calculate();
}

void ViewerWidget::resetCurrentFractal() {
  renderer->set_coords(renderer->initial_coords(), viewport);
  calculate();
}

void ViewerWidget::changeFractal(const fractals::PointwiseFractal &fractal) {
  renderer->set_fractal(fractal);
  renderer->set_coords(renderer->initial_coords(), viewport);
  renderer->redraw(viewport);
  calculate();
}

std::vector<std::pair<std::string, const fractals::PointwiseFractal &>>
ViewerWidget::listFractals() {
  auto registry = fractals::make_registry();
  register_fractals(*registry);
  return registry->listFractals();
}

void ViewerWidget::enableAutoDepth(bool value) {
  renderer->enable_auto_depth(value);
}

void ViewerWidget::enableThreading(bool value) {
  // TODO: Configure this a bit better
  renderer->set_threading(value ? 4 : 1);
}

void ViewerWidget::quickSave() {
  auto home = getenv("HOME");
  if (home) {
    auto desktop = std::filesystem::path{home} / "Desktop";
    auto prefix = "fractal ";
    std::filesystem::path image_filename;

    for (int file_counter = 0; file_counter < 1000; ++file_counter) {
      std::stringstream ss;
      ss << (desktop / prefix).string();
      ss << file_counter << ".png";
      image_filename = ss.str();
      if (!std::filesystem::exists(image_filename))
        break;
    }

    // Update the image metadata
    auto coords = renderer->get_coords();
    int zeros = fractals::count_fractional_zeros(coords.r);
    int width = 4 + zeros * 0.30103;

    std::stringstream ss;
    ss << "Generated by Mandelbrot-Qt\n";
    ss << "X: " << std::setprecision(width) << coords.x << std::endl;
    ss << "Y: " << coords.y << std::endl;
    ss << "R: " << coords.r << std::endl;
    ss << "Max iterations: " << coords.max_iterations << std::endl;

    image.setText("Description", ss.str().c_str());
    image.save((image_filename).string().c_str(), "png");
  }
}

void ViewerWidget::scalePalette() {
  double min, p, max;
  renderer->get_depth_range(min, p, max);
  if (p > 0)
    colourMap->setRange(min, p);
  else if (max > 0)
    colourMap->setRange(min, max);
  renderer->redraw(viewport);
  calculate();
}
