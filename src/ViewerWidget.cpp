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

#include "RenderingMetrics.hpp"
#include "fractal_calculation.hpp"
#include "json.hpp"
#include "mandelbrot.hpp"
#include "nlohmann/json.hpp"
#include "view_parameters.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

ViewerWidget::ViewerWidget(QWidget *parent)
    : QWidget{parent}, viewport(*this), renderer(viewport), controlPanel(this) {
  setFastAnimation();

  renderingTimer.setSingleShot(true);
  connect(&renderingTimer, &QTimer::timeout, this, &ViewerWidget::updateFrame);
  // Called on a separate thread so we can't just start new work
  connect(this, &ViewerWidget::renderingFinishedSignal, this,
          &ViewerWidget::renderingFinishedSlot);

  connect(&controlPanel, &ControlPanel::updateParameters, this,
          &ViewerWidget::shadingParametersChanged);
  connect(&controlPanel, &ControlPanel::rescalePalette, this,
      &ViewerWidget::scalePalette);
}

void ViewerWidget::paintEvent(QPaintEvent *event) { draw(); }

void ViewerWidget::calculate() {
  startCalculating(renderer.renderer->log_width(),
                   renderer.renderer->iterations());

  assert(image.width() > 0);

  // viewport.init(image.width(), image.height(), (fractals::RGB
  // *)image.bits());

  renderer.calculate_async();
  // calculateFlagLocations();
}

void ViewerWidget::draw() {
  pending_redraw = 0;
  QPainter painter(this);

  auto &colourMap = *renderer.colourMap;

  std::uint32_t *image_data = (std::uint32_t *)image.bits();
  for (int j = 0; j < image.height(); ++j) {
    for (int i = 0; i < image.width(); ++i) {
      int delta = 1;
      auto &pixel = viewport(i, j);
      auto &p2 = i + delta < image.width() ? viewport(i + delta, j)
                                           : viewport(i - delta, j);
      auto &p3 = j + delta < image.height() ? viewport(i, j + delta)
                                            : viewport(i, j - delta);
      constexpr bool alwaysShade = true;
      if ((pixel.error == 0 && p2.error == 0 && p3.error == 0) || alwaysShade) {
        double dx = i + delta < image.width() ? p2.value - viewport(i, j).value
                                              : viewport(i, j).value - p2.value;
        double dy = j + delta < image.height()
                        ? p3.value - viewport(i, j).value
                        : viewport(i, j).value - p3.value;
        image_data[j * image.width() + i] =
            0xff000000 | colourMap(pixel.value, dx, dy);
      } else {
        image_data[j * image.width() + i] =
            0xff000000 | colourMap(viewport(i, j).value);
      }
    }
  }

  painter.drawImage(this->rect(), image);
}

void ViewerWidget::doResize(int w, int h) {
  w *= imageScale;
  h *= imageScale;

  // Should stop the current calculation
  renderer.renderer->set_aspect_ratio(w, h);

  viewport.init(w, h);
  image = QImage(w, h, QImage::Format_RGB32);
  std::fill(viewport.begin(), viewport.end(), viewport.invalid_value());
  pending_resize = false;
}

void ViewerWidget::resizeEvent(QResizeEvent *event) {
  if (renderer.is_animating()) {
    pending_resize = true;
    return;
  }
  doResize(event->size().width(), event->size().height());
  renderer.cancel_animations();
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
    renderer.cancel_animations();
    renderer.renderer->zoom(r, imageScale * event->position().x(),
                            imageScale * event->position().y(), false,
                            viewport);
    calculate();
  }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  int x = event->pos().x() * imageScale;
  int y = event->pos().y() * imageScale;
  if (event->buttons() & Qt::LeftButton) {
    renderer.cancel_animations();
    renderer.renderer->scroll(press_x - x, press_y - y, viewport);
    calculate();
    press_x = x;
    press_y = y;
  }
  move_x = x;
  move_y = y;
  renderer.set_cursor(move_x, move_y);
}

void ViewerWidget::autoZoom() { renderer.auto_navigate(); }

void ViewerWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    int x = event->pos().x() * imageScale;
    int y = event->pos().y() * imageScale;
    start_x = press_x = x;
    start_y = press_y = y;

    release_can_start_zooming = !renderer.is_animating();
    stopAnimations();
  }
}

void ViewerWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (release_can_start_zooming && start_x == move_x && start_y == move_y) {
      zoomAtCursor();
    }
  }
}

void ViewerWidget::doUpdate() {
  update();
}

void ViewerWidget::MyViewport::updated() {
  // Note will be called on different threads
  if (!widget.pending_redraw) {
    ++widget.pending_redraw;
    widget.doUpdate();
  }
}

void ViewerWidget::MyViewport::calculation_started(double logRadius,
                                                   int iterations) {
  widget.startCalculating(logRadius, iterations);
}

void ViewerWidget::MyViewport::schedule_next_calculation() {
  widget.renderingFinishedSignal();
}

void ViewerWidget::MyViewport::finished(
    const fractals::RenderingMetrics &metrics) {

  if (widget.renderer.renderer)
    widget.renderer.discovered_depth(metrics);
  widget.setSpeedEstimate(metrics.seconds_per_point);

  if (metrics.fully_evaluated)
    widget.completed(&metrics);

  if (widget.renderer.current_animation ==
      fractals::AnimatedRenderer::AnimationType::startzoomtopoint) {
    widget.renderer.current_animation =
        fractals::AnimatedRenderer::AnimationType::zoomtopoint;
    widget.renderingFinishedSignal();
  }
}

void ViewerWidget::increaseIterations() {
  renderer.cancel_animations();
  renderer.renderer->increase_iterations(viewport);
  calculate();
}

void ViewerWidget::decreaseIterations() {
  renderer.cancel_animations();
  renderer.renderer->decrease_iterations(viewport);
  calculate();
}

void ViewerWidget::pasteCoords() {
  QClipboard *clipboard = QApplication::clipboard();
  auto t = clipboard->text();
  std::cout << "Pasted: " << t.toStdString() << std::endl;
}

void ViewerWidget::copyCoords() {

  fractals::view_parameters params;
  getCoords(params);
  auto js = write_json(params);
  std::string str = js.dump(4);

  QClipboard *clipboard = QApplication::clipboard();

  QMimeData *data = new QMimeData;
  data->setImageData(image);
  data->setText(str.c_str());
  clipboard->setMimeData(data);
}

void ViewerWidget::getCoords(fractals::view_parameters &params) const {
  renderer.renderer->save(params);
  renderer.colourMap->save(params);
}

bool ViewerWidget::setCoords(const fractals::view_parameters &params) {
  renderer.cancel_animations();
  renderer.colourMap->load(params);
  renderer.renderer->load(params, viewport);
  calculate();
  return true;
}

void ViewerWidget::recolourPalette() {
  renderer.colourMap->randomize();
  updateColourControls();
  update();
}

void ViewerWidget::resetCurrentFractal() {
  renderer.cancel_animations();
  renderer.renderer->set_coords(renderer.renderer->initial_coords(), viewport);
  renderer.colourMap->resetGradient();
  updateColourControls();
  calculate();
}

void ViewerWidget::changeFractal(const fractals::fractal &fractal) {
  renderer.cancel_animations();

  std::string old_family = renderer.renderer->get_fractal_family();
  renderer.renderer->set_fractal(fractal);

  if (old_family != fractal.family())
    renderer.renderer->set_coords(renderer.renderer->initial_coords(),
                                  viewport);
  renderer.renderer->redraw(viewport);
  calculate();
}

std::vector<std::pair<std::string, const fractals::fractal &>>
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

  image.setText("MandelbrotQtjson", write_json(params).dump().c_str());
  image.save(image_filename, "png");
}

void ViewerWidget::scalePalette() {
  double min, p, max;
  renderer.renderer->get_depth_range(min, p, max);
  if (p > 0)
    renderer.colourMap->setRange(min, p);
  else if (max > 0)
    renderer.colourMap->setRange(min, max);
  updateColourControls();
  update();
}

void ViewerWidget::open() {
  auto str =
      QFileDialog::getOpenFileName(this, "Open file", {}, "Images (*.png)");
  if (!str.isEmpty()) {
    QImage image;
    image.load(str);
    auto text = image.text("MandelbrotQt");
    fractals::view_parameters params;
    if (!text.isEmpty()) {
      // Successfully loaded metadata
      std::stringstream ss(text.toStdString());
      ss >> params;
    } else {
      text = image.text("MandelbrotQtjson");
      if (!text.isEmpty()) {
        params = read_json(nlohmann::json::parse(text.toStdString()));
      } else
        return;
    }

    renderer.cancel_animations();

    renderer.renderer->load(params, viewport);
    renderer.colourMap->load(params);
    fractalChanged(renderer.renderer->get_fractal_name()
                       .c_str()); // Update menus if needed
    calculate();
  }
}

void ViewerWidget::openBookmark(const fractals::view_parameters *params) {
  renderer.cancel_animations();

  renderer.renderer->load(*params, viewport);
  renderer.colourMap->load(*params);
  fractalChanged(
      renderer.renderer->get_fractal_name().c_str()); // Update menus if needed
  controlPanel.valuesChanged(&params->shader);
  calculate();
}

void ViewerWidget::save() {
  auto str = QFileDialog::getSaveFileName(this, "Save file", "fractal.png",
                                          "PNG (*.png)");
  if (!str.isEmpty()) {
    saveToFile(str);
  }
}

void ViewerWidget::zoomIn() {
  renderer.cancel_animations();
  renderer.renderer->zoom(0.5, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::smoothZoomIn() { renderer.smooth_zoom_in(); }

void ViewerWidget::updateFrame() {
  // calculateFlagLocations();
  renderer.timer();
}

void ViewerWidget::MyViewport::start_timer() {
  widget.renderingTimer.start(10);
}

void ViewerWidget::zoomOut() {
  renderer.cancel_animations();
  renderer.renderer->zoom(2.0, move_x, move_y, false, viewport);
  calculate();
}

void ViewerWidget::animateToHere() {
  renderer.animate_to_here();
  calculate();
}

void ViewerWidget::zoomAtCursor() { renderer.zoom_at_cursor(); }

void ViewerWidget::setSpeedEstimate(double secondsPerPixel) {
  renderer.set_speed_estimate(secondsPerPixel);
}

void ViewerWidget::renderingFinishedSlot() {
  // Is a resize pending?
  if (pending_resize) {
    // We don't resize the image whilst animating, but we can do it
    // in between animation frames
    doResize(width(), height());
  }

  renderer.start_next_calculation();
}

void ViewerWidget::stopAnimations() {
  renderer.cancel_animations();
  calculate();
}

void ViewerWidget::setQualityAnimation() {
  renderer.set_animation_speed(750ms, false);
}

void ViewerWidget::setFastAnimation() {
  renderer.set_animation_speed(750ms, true);
}

void ViewerWidget::setFastestAnimation() {
  renderer.set_animation_speed(50ms, true);
}

ViewerWidget::MyViewport::MyViewport(ViewerWidget &widget) : widget(widget) {}

void ViewerWidget::MyViewport::stop_timer() { widget.renderingTimer.stop(); }

void ViewerWidget::enableOversampling(bool checked) {
  QResizeEvent s(size(), size());
  imageScale = checked ? 2.0 : 1.0;
  resizeEvent(&s);
}

void ViewerWidget::enableAutoGradient(bool checked) {
  if (checked)
    renderer.enable_auto_gradient();
  else
    renderer.disable_auto_gradient();
  update();
}

void ViewerWidget::enableShading(bool checked) {
  auto &colourMap = *renderer.colourMap;
  fractals::shader_parameters params;
  colourMap.getParameters(params);
  params.shading = checked;
  colourMap.setParameters(params);
  controlPanel.valuesChanged(&params);
  shadingChanged(checked);
  update();
}

void ViewerWidget::showOptions() {
  updateColourControls();
  controlPanel.show();
}

void ViewerWidget::shadingParametersChanged(const fractals::shader_parameters *params) {
  renderer.colourMap->setParameters(*params);
  shadingChanged(params->shading);
  update();
}

void ViewerWidget::updateColourControls() {
  auto &colourMap = *renderer.colourMap;
  fractals::shader_parameters params;
  colourMap.getParameters(params);
  controlPanel.valuesChanged(&params);
}
