#include "ViewerWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QImage>
#include <QMimeData>
#include <QMoveEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QWheelEvent>

#include "mandelbrot.hpp"
#include "registry.hpp"
#include "view_parameters.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>

void register_fractals(fractals::Registry &r);

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()},
      renderer{fractals::make_renderer(*registry)} {

  register_fractals(*registry);

  renderingTimer.setSingleShot(true);
  connect(&renderingTimer, &QTimer::timeout, this, &ViewerWidget::updateFrame);
}

ViewerWidget::~ViewerWidget() { renderer.reset(); }

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  startCalculating(renderer->log_width(), renderer->iterations());

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
  // Should stop the current calculation
  renderer->set_aspect_ratio(event->size().width(), event->size().height());

  viewport.invalidateAllPixels();
  image = QImage(event->size(), QImage::Format_RGB32);
  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();
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

void ViewerWidget::copyCoords() {
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

void ViewerWidget::getCoords(fractals::view_parameters &params) const {
  renderer->save(params);
  colourMap->save(params);
}

bool ViewerWidget::setCoords(const fractals::view_parameters &params) {
  colourMap->load(params);
  renderer->load(params, viewport);
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
  std::string old_family = renderer->get_fractal_family();
  renderer->set_fractal(fractal);

  if (old_family != fractal.family())
    renderer->set_coords(renderer->initial_coords(), viewport);
  renderer->redraw(viewport);
  calculate();
}

std::vector<std::pair<std::string, const fractals::PointwiseFractal &>>
ViewerWidget::listFractals() {
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
  auto desktop_list =
      QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);

  if (!desktop_list.empty()) {
    auto desktop = std::filesystem::path{desktop_list[0].toStdString()};
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
    saveToFile((image_filename).string().c_str());
  }
}

void ViewerWidget::saveToFile(const QString &image_filename) {
  fractals::view_parameters params;
  renderer->save(params);
  colourMap->save(params);

  std::stringstream ss;
  ss << params;

  image.setText("MandelbrotQt", ss.str().c_str());
  image.save(image_filename, "png");
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

void ViewerWidget::open() {
  auto str =
      QFileDialog::getOpenFileName(this, "Open file", {}, "Images (*.png)");
  if (!str.isEmpty()) {
    QImage image;
    image.load(str);
    auto text = image.text("MandelbrotQt");
    if (!text.isEmpty()) {
      // Successfully loaded metadata
      std::stringstream ss(text.toStdString());
      fractals::view_parameters params;
      ss >> params;
      renderer->load(params, viewport);
      colourMap->load(params);
      calculate();
    } else {
      // TODO: Pop up a dialog box
    }
  }
}

void ViewerWidget::save() {
  auto str = QFileDialog::getSaveFileName(this, "Save file", "fractal.png",
                                          "PNG (*.png)");
  if (!str.isEmpty()) {
    saveToFile(str);
  }
}

void ViewerWidget::center() {
  renderer->center(viewport);
  calculate();
}

void ViewerWidget::zoomIn() {
  renderer->zoom(0.5, move_x, move_y, viewport);
  calculate();
}

void ViewerWidget::smoothZoomIn() {

  if (!zooming) {

    zooming = true;
    calculationFinished = false;
    computedImage = image;

    previousImage = image;
    using namespace std::literals::chrono_literals;
    zoom_start = std::chrono::system_clock::now();
    zoom_duration = 500ms;

    assert(computedImage.width() > 0);

    background_viewport.widget = this;
    background_viewport.data = (fractals::RGB *)computedImage.bits();
    background_viewport.width = computedImage.width();
    background_viewport.height = computedImage.height();

    renderer->zoom(0.5, move_x, move_y, background_viewport);
    startCalculating(renderer->log_width(), renderer->iterations());
    renderer->calculate_async(background_viewport, *colourMap);
    renderingTimer.start(10);
  }
}

void ViewerWidget::updateFrame() {
  auto now = std::chrono::system_clock::now();
  double time_ratio =
      std::chrono::duration<double>(now - zoom_start) / zoom_duration;
  std::cout << "Time to update " << (100 * time_ratio) << "%\n";
  if (time_ratio >= 1) {
    // Maybe carry on zooming to the next frame
    if (calculationFinished)
      renderFinished2();
    else {
      // Project the current view into the frame
      auto zoom_ratio = std::pow(0.5, time_ratio);
    }
  } else {
    // Update the current view using the
    // The scaling ratio isn't actually linear !!

    renderingTimer.start(10);
  }
}

void ViewerWidget::BackgroundViewport::region_updated(int x, int y, int w,
                                                      int h) {}

void ViewerWidget::BackgroundViewport::finished(double width, int min_depth,
                                                int max_depth, double avg,
                                                double skipped,
                                                double render_time) {
  widget->backgroundRenderFinished();
}

void ViewerWidget::renderFinished2() {
  std::copy(background_viewport.data,
            background_viewport.data +
                background_viewport.width * background_viewport.height,
            viewport.data);
  update();
}

void ViewerWidget::backgroundRenderFinished() {
  zooming = false;
  calculationFinished = true;

  // !! This test is fragile

  auto now = std::chrono::system_clock::now();
  if (std::chrono::duration<double>(now - zoom_start) >= zoom_duration) {
    renderFinished2();
  }
}

void ViewerWidget::BackgroundViewport::discovered_depth(
    int points, double discovered_depth) {}

void ViewerWidget::zoomOut() {
  renderer->zoom(2.0, move_x, move_y, viewport);
  calculate();
}

void ViewerWidget::autoZoom() {
  renderer->auto_step(viewport);
  calculate();
}

void ViewerWidget::autoZoomContinue() {
  renderer->auto_step_continue(viewport);
  calculate();
}
