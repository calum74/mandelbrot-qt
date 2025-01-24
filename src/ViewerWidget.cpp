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
#include <thread>

using namespace std::literals::chrono_literals;

void register_fractals(fractals::Registry &r);

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()},
      renderer{fractals::make_renderer(*registry)} {
  setFastAnimation();

  register_fractals(*registry);

  renderingTimer.setSingleShot(true);
  connect(&renderingTimer, &QTimer::timeout, this, &ViewerWidget::updateFrame);
  // Called on a separate thread so we can't just start new work
  connect(this, &ViewerWidget::renderingFinishedSignal, this,
          &ViewerWidget::renderingFinishedSlot);
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
  cancelAnimations();
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
    cancelAnimations();
    renderer->zoom(r, event->position().x(), event->position().y(), false,
                   viewport);
    calculate();
  }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    cancelAnimations();
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

    stopAnimations();
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
  widget->renderingFinishedSignal();
}

void ViewerWidget::MyViewport::discovered_depth(int points,
                                                double discovered_depth,
                                                double time) {
  if (widget->renderer)
    widget->renderer->discovered_depth(points, discovered_depth);
  widget->setSpeedEstimate(time);
}

void ViewerWidget::increaseIterations() {
  cancelAnimations();
  renderer->increase_iterations(viewport);
  calculate();
}

void ViewerWidget::decreaseIterations() {
  cancelAnimations();
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
  cancelAnimations();
  colourMap->load(params);
  renderer->load(params, viewport);
  calculate();
  return true;
}

void ViewerWidget::recolourPalette() {
  cancelAnimations();
  colourMap->randomize();
  renderer->redraw(viewport);
  calculate();
}

void ViewerWidget::resetCurrentFractal() {
  cancelAnimations();
  renderer->set_coords(renderer->initial_coords(), viewport);
  colourMap->resetGradient();
  calculate();
}

void ViewerWidget::changeFractal(const fractals::PointwiseFractal &fractal) {
  cancelAnimations();

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
  if (value)
    renderer->set_threading(4);
}

void ViewerWidget::singleThreaded(bool value) {
  if (value)
    renderer->set_threading(1);
}

void ViewerWidget::maxThreading(bool value) {
  if (value)
    renderer->set_threading(std::thread::hardware_concurrency());
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
  cancelAnimations();
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
      cancelAnimations();

      renderer->load(params, viewport);
      colourMap->load(params);
      fractalChanged(renderer->get_fractal_name()); // Update menus if needed
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

void ViewerWidget::zoomIn() {
  cancelAnimations();
  renderer->zoom(0.5, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::smoothZoomTo(int x, int y, bool lockCenter) {
  zooming = true;
  calculationFinished = false;
  zoomTimeout = false;
  computedImage = image;
  zoom_x = x;
  zoom_y = y;

  previousImage = image;
  zoom_start = std::chrono::system_clock::now();
  // Add a 5% buffer to reduce stuttering
  zoom_duration = std::chrono::milliseconds(
      int(estimatedSecondsPerPixel * 1000 * viewport.width * viewport.height *
          1.05)); // Stupid stupid std::chrono

  // Stop the zoom duration getting too out of hand
  if (zoom_duration < 750ms)
    zoom_duration = 750ms;

  if (fixZoomSpeed)
    zoom_duration = fixZoomDuration; // Override for speed

  assert(computedImage.width() > 0);

  background_viewport.widget = this;
  background_viewport.data = (fractals::RGB *)computedImage.bits();
  background_viewport.width = computedImage.width();
  background_viewport.height = computedImage.height();

  renderer->zoom(0.5, zoom_x, zoom_y, lockCenter, background_viewport);
  renderer->calculate_async(background_viewport, *colourMap);
  renderingTimer.start(10);
}

void ViewerWidget::smoothZoomIn() {
  cancelAnimations();
  if (!zooming) {
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::updateFrame() {
  if (!zooming)
    return;

  auto now = std::chrono::system_clock::now();
  double time_ratio =
      std::chrono::duration<double>(now - zoom_start) / zoom_duration;
  if (time_ratio >= 1) {
    zoomTimeout = true;
    // Maybe carry on zooming to the next frame
    if (calculationFinished || fixZoomSpeed) {
      renderFinishedBackgroundImage();
      beginNextAnimation();
    } else {
      // It's taking some time, so update the status bar
      startCalculating(renderer->log_width(), renderer->iterations());
    }
  } else {
    // Update the current view using the
    // The scaling ratio isn't actually linear !!
    // Project the current view into the frame
    auto zoom_ratio = std::pow(0.5, time_ratio);
    fractals::Viewport previousVp;
    previousVp.data = (fractals::RGB *)previousImage.bits();
    previousVp.width = previousImage.width();
    previousVp.height = previousImage.height();

    fractals::map_viewport(previousVp, viewport, zoom_x * (1 - zoom_ratio),
                           zoom_y * (1 - zoom_ratio), zoom_ratio);
    update();

    renderingTimer.start(10);
  }
}

void ViewerWidget::BackgroundViewport::region_updated(int x, int y, int w,
                                                      int h) {}

void ViewerWidget::BackgroundViewport::finished(double width, int min_depth,
                                                int max_depth, double avg,
                                                double skipped,
                                                double render_time) {
  // if (!widget->zooming)
  //   return;
  widget->backgroundRenderFinished();
  widget->completed(width, min_depth, max_depth, avg, skipped, render_time);
}

void ViewerWidget::renderFinishedBackgroundImage() {
  // if (!zooming)
  //    return;
  std::copy(background_viewport.data,
            background_viewport.data +
                background_viewport.width * background_viewport.height,
            viewport.data);
  update();
}

void ViewerWidget::backgroundRenderFinished() {
  calculationFinished = true;

  if (zoomTimeout) {
    renderFinishedBackgroundImage();
    zooming = false;
    beginNextAnimation();
  }
}

void ViewerWidget::beginNextAnimation() {
  if (!calculationFinished) {
    // Report on current calculation
    startCalculating(renderer->log_width(), renderer->iterations());
  }

  renderingFinishedSignal();
}

void ViewerWidget::BackgroundViewport::discovered_depth(
    int points, double discovered_depth, double seconds_per_pixel) {
  if (widget->renderer)
    widget->renderer->discovered_depth(points, discovered_depth);
  widget->setSpeedEstimate(seconds_per_pixel);
}

void ViewerWidget::zoomOut() {
  cancelAnimations();
  renderer->zoom(2.0, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::autoZoom() {
  cancelAnimations();
  int x, y;
  if (renderer->get_auto_zoom(x, y)) {
    current_animation = AnimationType::autozoom;
    smoothZoomTo(x, y, false);
  } else {
    std::cout << "Autozoom continue failed\n";
  }
}

void ViewerWidget::autoZoomContinue() {
  //  renderer->auto_step_continue(viewport);
  //  calculate();
}

void ViewerWidget::cancelAnimations() {
  renderingTimer.stop();
  current_animation = AnimationType::none;
  if (zooming) {
    renderFinishedBackgroundImage();
    zooming = false;
  }
}

void ViewerWidget::animateToHere() {
  cancelAnimations();
  current_animation = AnimationType::zoomtopoint;
  auto c = renderer->get_coords();
  c.r = 2.0;
  c.max_iterations = 500;
  zoomtopoint_limit = renderer->log_width();
  renderer->set_coords(c, viewport);
  calculate();
}

void ViewerWidget::zoomAtCursor() {
  if (zooming) {
    cancelAnimations();
  } else {
    cancelAnimations();
    current_animation = AnimationType::zoomatcursor;
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::setSpeedEstimate(double secondsPerPixel) {
  estimatedSecondsPerPixel = secondsPerPixel;
}

void ViewerWidget::renderingFinishedSlot() {
  // !! switch statement
  if (current_animation == AnimationType::autozoom) {
    autoZoom();
  } else if (current_animation == AnimationType::zoomtopoint) {
    if (renderer->log_width() > zoomtopoint_limit)
      smoothZoomTo(viewport.width / 2, viewport.height / 2, true);
  } else if (current_animation == AnimationType::zoomatcursor) {
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::stopAnimations() { cancelAnimations(); }

void ViewerWidget::setQualityAnimation() { fixZoomSpeed = false; }

void ViewerWidget::setFastAnimation() {
  fixZoomSpeed = true;
  fixZoomDuration = 750ms;
}

void ViewerWidget::setFastestAnimation() {
  fixZoomSpeed = true;
  fixZoomDuration = 50ms;
}
