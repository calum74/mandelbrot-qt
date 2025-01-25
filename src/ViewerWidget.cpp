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
#include "view_parameters.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

ViewerWidget::ViewerWidget(QWidget *parent) : QWidget{parent} {
  setFastAnimation();

  renderingTimer.setSingleShot(true);
  connect(&renderingTimer, &QTimer::timeout, this, &ViewerWidget::updateFrame);
  // Called on a separate thread so we can't just start new work
  connect(this, &ViewerWidget::renderingFinishedSignal, this,
          &ViewerWidget::renderingFinishedSlot);
}

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  startCalculating(renderer.renderer->log_width(),
                   renderer.renderer->iterations());

  assert(image.width() > 0);

  viewport.widget = this;
  viewport.data = (fractals::RGB *)image.bits();
  viewport.width = image.width();
  viewport.height = image.height();

  renderer.calculate_async(viewport);
}

void ViewerWidget::draw() {
  pending_redraw = 0;
  QPainter painter(this);

  painter.drawImage(this->rect(), image);
}

void ViewerWidget::resizeEvent(QResizeEvent *event) {
  cancelAnimations();

  // Should stop the current calculation
  renderer.renderer->set_aspect_ratio(event->size().width(),
                                      event->size().height());

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
  // Since each unit of scroll magnifies the image, using std::pow will give
  // the correct composition of 2 scrolls. 0.992 is a fudge factor, decrease it
  // for a faster zoom.
  double r = std::pow(0.992, event->angleDelta().y());
  if (r > 2.0)
    r = 2.0;
  if (r < 0.5)
    r = 0.5;
  if (r != 1.0) {
    cancelAnimations();
    renderer.renderer->zoom(r, event->position().x(), event->position().y(),
                            false, viewport);
    calculate();
  }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    cancelAnimations();
    renderer.renderer->scroll(press_x - event->pos().x(),
                              press_y - event->pos().y(), viewport);
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

void ViewerWidget::MyViewport::calculation_started(double logRadius,
                                                   int iterations) {
  widget->startCalculating(logRadius, iterations);
}

void ViewerWidget::MyViewport::schedule_next_calculation() {
  widget->renderingFinishedSignal();
}

void ViewerWidget::MyViewport::finished(double width, int min_depth,
                                        int max_depth, double avg,
                                        double skipped, double render_time) {
  widget->completed(width, min_depth, max_depth, avg, skipped, render_time);

  if (widget->renderer.current_animation ==
      AnimatedRenderer::AnimationType::startzoomtopoint) {
    widget->renderer.current_animation =
        AnimatedRenderer::AnimationType::zoomtopoint;
    widget->renderingFinishedSignal();
  }
}

void ViewerWidget::MyViewport::discovered_depth(int points,
                                                double discovered_depth,
                                                double time) {
  if (widget->renderer.renderer)
    widget->renderer.renderer->discovered_depth(points, discovered_depth);
  widget->setSpeedEstimate(time);
}

void ViewerWidget::increaseIterations() {
  cancelAnimations();
  renderer.renderer->increase_iterations(viewport);
  calculate();
}

void ViewerWidget::decreaseIterations() {
  cancelAnimations();
  renderer.renderer->decrease_iterations(viewport);
  calculate();
}

void ViewerWidget::copyCoords() {
  auto c = renderer.renderer->get_coords();

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
  renderer.renderer->save(params);
  renderer.colourMap->save(params);
}

bool ViewerWidget::setCoords(const fractals::view_parameters &params) {
  cancelAnimations();
  renderer.colourMap->load(params);
  renderer.renderer->load(params, viewport);
  calculate();
  return true;
}

void ViewerWidget::recolourPalette() {
  cancelAnimations();
  renderer.colourMap->randomize();
  renderer.renderer->redraw(viewport);
  calculate();
}

void ViewerWidget::resetCurrentFractal() {
  cancelAnimations();
  renderer.renderer->set_coords(renderer.renderer->initial_coords(), viewport);
  renderer.colourMap->resetGradient();
  calculate();
}

void ViewerWidget::changeFractal(const fractals::PointwiseFractal &fractal) {
  cancelAnimations();

  std::string old_family = renderer.renderer->get_fractal_family();
  renderer.renderer->set_fractal(fractal);

  if (old_family != fractal.family())
    renderer.renderer->set_coords(renderer.renderer->initial_coords(),
                                  viewport);
  renderer.renderer->redraw(viewport);
  calculate();
}

std::vector<std::pair<std::string, const fractals::PointwiseFractal &>>
ViewerWidget::listFractals() {
  return renderer.registry->listFractals();
}

void ViewerWidget::enableAutoDepth(bool value) {
  renderer.renderer->enable_auto_depth(value);
}

void ViewerWidget::enableThreading(bool value) {
  if (value)
    renderer.renderer->set_threading(4);
}

void ViewerWidget::singleThreaded(bool value) {
  if (value)
    renderer.renderer->set_threading(1);
}

void ViewerWidget::maxThreading(bool value) {
  if (value)
    renderer.renderer->set_threading(std::thread::hardware_concurrency());
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
  renderer.renderer->save(params);
  renderer.colourMap->save(params);

  std::stringstream ss;
  ss << params;

  image.setText("MandelbrotQt", ss.str().c_str());
  image.save(image_filename, "png");
}

void ViewerWidget::scalePalette() {
  cancelAnimations();
  double min, p, max;
  renderer.renderer->get_depth_range(min, p, max);
  if (p > 0)
    renderer.colourMap->setRange(min, p);
  else if (max > 0)
    renderer.colourMap->setRange(min, max);
  renderer.renderer->redraw(viewport);
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

      renderer.renderer->load(params, viewport);
      renderer.colourMap->load(params);
      fractalChanged(
          renderer.renderer->get_fractal_name()); // Update menus if needed
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
  renderer.renderer->zoom(0.5, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::smoothZoomTo(int x, int y, bool lockCenter) {
  renderer.smoothZoomTo(x, y, lockCenter);
  renderingTimer.start(10);
}

void ViewerWidget::smoothZoomIn() {
  cancelAnimations();
  if (!renderer.zooming) {
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::updateFrame() {
  if (!renderer.zooming)
    return;

  auto now = std::chrono::system_clock::now();
  double time_ratio = std::chrono::duration<double>(now - renderer.zoom_start) /
                      renderer.zoom_duration;
  if (time_ratio >= 1) {
    renderer.zoomTimeout = true;
    // Maybe carry on zooming to the next frame
    if (renderer.calculationFinished || renderer.fixZoomSpeed) {
      renderer.renderFinishedBackgroundImage();
      renderer.beginNextAnimation();
    } else {
      // It's taking some time, so update the status bar
      startCalculating(renderer.renderer->log_width(),
                       renderer.renderer->iterations());
    }
  } else {
    // Update the current view using the
    // The scaling ratio isn't actually linear !!
    // Project the current view into the frame
    auto zoom_ratio = std::pow(0.5, time_ratio);
    fractals::Viewport previousVp;
    previousVp.data = renderer.previousImageData.data();
    previousVp.width = viewport.width;
    previousVp.height = viewport.height;

    fractals::map_viewport(previousVp, viewport,
                           renderer.zoom_x * (1 - zoom_ratio),
                           renderer.zoom_y * (1 - zoom_ratio), zoom_ratio);
    update();

    renderingTimer.start(10);
  }
}

void ViewerWidget::MyViewport::start_timer() {
  widget->renderingTimer.start(10);
}

void ViewerWidget::zoomOut() {
  cancelAnimations();
  renderer.renderer->zoom(2.0, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::autoZoom() {
  cancelAnimations();
  int x, y;
  if (renderer.renderer->get_auto_zoom(x, y)) {
    renderer.current_animation = AnimatedRenderer::AnimationType::autozoom;
    smoothZoomTo(x, y, false);
  } else {
    std::cout << "Autozoom continue failed\n";
  }
}

void ViewerWidget::cancelAnimations() {
  renderingTimer.stop();
  renderer.current_animation = AnimatedRenderer::AnimationType::none;
  if (renderer.zooming) {
    renderer.renderFinishedBackgroundImage();
    renderer.zooming = false;
  }
}

void ViewerWidget::animateToHere() {
  cancelAnimations();
  renderer.current_animation =
      AnimatedRenderer::AnimationType::startzoomtopoint;
  auto c = renderer.renderer->get_coords();
  c.r = 2.0;
  c.max_iterations = 500;
  renderer.zoomtopoint_limit = renderer.renderer->log_width();
  renderer.renderer->set_coords(c, viewport);
  calculate();
}

void ViewerWidget::zoomAtCursor() {
  if (renderer.zooming) {
    cancelAnimations();
  } else {
    cancelAnimations();
    renderer.current_animation = AnimatedRenderer::AnimationType::zoomatcursor;
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::setSpeedEstimate(double secondsPerPixel) {
  renderer.estimatedSecondsPerPixel = secondsPerPixel;
}

void ViewerWidget::renderingFinishedSlot() {
  // !! switch statement
  if (renderer.current_animation == AnimatedRenderer::AnimationType::autozoom) {
    autoZoom();
  } else if (renderer.current_animation ==
             AnimatedRenderer::AnimationType::zoomtopoint) {
    if (renderer.renderer->log_width() > renderer.zoomtopoint_limit)
      smoothZoomTo(viewport.width / 2, viewport.height / 2, true);
  } else if (renderer.current_animation ==
             AnimatedRenderer::AnimationType::zoomatcursor) {
    smoothZoomTo(move_x, move_y, false);
  }
}

void ViewerWidget::stopAnimations() { cancelAnimations(); }

void ViewerWidget::setQualityAnimation() { renderer.fixZoomSpeed = false; }

void ViewerWidget::setFastAnimation() {
  renderer.fixZoomSpeed = true;
  renderer.fixZoomDuration = 750ms;
}

void ViewerWidget::setFastestAnimation() {
  renderer.fixZoomSpeed = true;
  renderer.fixZoomDuration = 50ms;
}
