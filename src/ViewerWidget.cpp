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

#include "calculation_metrics.hpp"
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
    : QWidget{parent}, renderer(*this), controlPanel(this) {
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
  // !! Delete this??
  startCalculating(renderer.ln_r(), renderer.iterations());

  assert(image.width() > 0);
  renderer.calculate_async();
}

void ViewerWidget::draw() {
  pending_redraw = 0;
  QPainter painter(this);

  auto &colourMap = *renderer.colourMap;

  std::uint32_t *image_data = (std::uint32_t *)image.bits();
  auto &values = renderer.view.values();

  for (int j = 0; j < image.height(); ++j) {
    for (int i = 0; i < image.width(); ++i) {
      int delta = 1;
      auto &pixel = values(i, j);
      auto &p2 = i + delta < image.width() ? values(i + delta, j)
                                           : values(i - delta, j);
      auto &p3 = j + delta < image.height()
                     ? values(i, j + delta)
                     : values(i, j - delta);
      constexpr bool alwaysShade = true;
      if ((pixel.error == 0 && p2.error == 0 && p3.error == 0) || alwaysShade) {
        double dx = i + delta < image.width()
                        ? p2.value - values(i, j).value
                        : values(i, j).value - p2.value;
        double dy = j + delta < image.height()
                        ? p3.value - values(i, j).value
                        : values(i, j).value - p3.value;
        image_data[j * image.width() + i] =
            0xff000000 | colourMap(pixel.value, dx, dy);
      } else {
        image_data[j * image.width() + i] =
            0xff000000 | colourMap(values(i, j).value);
      }
    }
  }

  painter.drawImage(this->rect(), image);
}

void ViewerWidget::doResize(int w, int h) {
  w *= imageScale;
  h *= imageScale;

  // Should stop the current calculation
  renderer.resize(w, h);

  image = QImage(w, h, QImage::Format_RGB32);
  pending_resize = false;
}

void ViewerWidget::resizeEvent(QResizeEvent *event) {
  if (renderer.is_animating()) {
    pending_resize = true;
    return;
  }
  doResize(event->size().width(), event->size().height());
  // renderer.cancel_animations();
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
    // renderer.cancel_animations();
    renderer.zoom(r, imageScale * event->position().x(),
                  imageScale * event->position().y(), false);
    // calculate();
  }
}

void ViewerWidget::mouseMoveEvent(QMouseEvent *event) {
  int x = event->pos().x() * imageScale;
  int y = event->pos().y() * imageScale;
  if (event->buttons() & Qt::LeftButton) {
    // renderer.cancel_animations();
    renderer.scroll(press_x - x, press_y - y);
    // calculate();
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

    if(renderer.is_animating())
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
  QWidget::update();
}

void ViewerWidget::values_changed() {
  // Note will be called on different threads
  if (!pending_redraw) {
    ++pending_redraw;
    doUpdate();
  }
}

void ViewerWidget::calculation_started(double logRadius, int iterations) {
  startCalculating(logRadius, iterations);
}

void ViewerWidget::calculation_finished(
    const fractals::calculation_metrics &metrics) {

  renderer.update_iterations(metrics);
  setSpeedEstimate(metrics.seconds_per_point);

  if (metrics.fully_evaluated)
    completed(&metrics);
}

void ViewerWidget::increaseIterations() {
  renderer.cancel_animations();
  renderer.increase_iterations();
  calculate();
}

void ViewerWidget::decreaseIterations() {
  renderer.cancel_animations();
  renderer.decrease_iterations();
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
  renderer.save(params);
  renderer.colourMap->save(params);
}

bool ViewerWidget::setCoords(const fractals::view_parameters &params) {
  renderer.load(params);
  return true;
}

void ViewerWidget::recolourPalette() {
  renderer.colourMap->randomize();
  updateColourControls();
  QWidget::update();
}

void ViewerWidget::resetCurrentFractal() {
  renderer.set_coords(renderer.initial_coords());
  renderer.colourMap->resetGradient();
  updateColourControls();
}

void ViewerWidget::changeFractal(const fractals::fractal &fractal) {

  std::string old_family = renderer.fractal_family();
  renderer.set_fractal(fractal, old_family != fractal.family());
}

std::vector<std::pair<std::string, const fractals::fractal &>>
ViewerWidget::listFractals() {
  return renderer.registry->listFractals();
}

void ViewerWidget::enableAutoDepth(bool value) {
  renderer.enable_auto_depth(value);
}

void ViewerWidget::enableThreading(bool value) {
  if (value)
    renderer.set_threading(4);
}

void ViewerWidget::singleThreaded(bool value) {
  if (value)
    renderer.set_threading(1);
}

void ViewerWidget::maxThreading(bool value) {
  if (value)
    renderer.set_threading(std::thread::hardware_concurrency());
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
  renderer.save(params);
  renderer.colourMap->save(params);

  image.setText("MandelbrotQtjson", write_json(params).dump().c_str());
  image.save(image_filename, "png");
}

void ViewerWidget::scalePalette() {
  double min, p, max;
  renderer.get_depth_range(min, p, max);
  if (p > 0)
    renderer.colourMap->setRange(min, p);
  else if (max > 0)
    renderer.colourMap->setRange(min, max);
  updateColourControls();
  QWidget::update();
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

    renderer.load(params);
    fractalChanged(renderer.fractal_name().c_str()); // Update menus if needed
  }
}

void ViewerWidget::openBookmark(const fractals::view_parameters *params) {
  renderer.load(*params);
  fractalChanged(renderer.fractal_name().c_str()); // Update menus if needed
  controlPanel.valuesChanged(&params->shader);
}

void ViewerWidget::save() {
  auto str = QFileDialog::getSaveFileName(this, "Save file", "fractal.png",
                                          "PNG (*.png)");
  if (!str.isEmpty()) {
    saveToFile(str);
  }
}

void ViewerWidget::zoomIn() {
  renderer.zoom(0.5, move_x, move_y, false);
}

void ViewerWidget::smoothZoomIn() { renderer.smooth_zoom_in(); }

void ViewerWidget::updateFrame() {}

void ViewerWidget::zoomOut() {
  renderer.zoom(2.0, move_x, move_y, false);
}

void ViewerWidget::animateToHere() {
  renderer.animate_to_here();
  // calculate();
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
  QWidget::update();
}

void ViewerWidget::enableShading(bool checked) {
  auto &colourMap = *renderer.colourMap;
  fractals::shader_parameters params;
  colourMap.getParameters(params);
  params.shading = checked;
  colourMap.setParameters(params);
  controlPanel.valuesChanged(&params);
  shadingChanged(checked);
  QWidget::update();
}

void ViewerWidget::showOptions() {
  updateColourControls();
  controlPanel.show();
}

void ViewerWidget::shadingParametersChanged(
    const fractals::shader_parameters *params) {
  renderer.colourMap->setParameters(*params);
  shadingChanged(params->shading);
  QWidget::update();
}

void ViewerWidget::updateColourControls() {
  auto &colourMap = *renderer.colourMap;
  fractals::shader_parameters params;
  colourMap.getParameters(params);
  controlPanel.valuesChanged(&params);
}

void ViewerWidget::animation_finished(
    const fractals::calculation_metrics &metrics) {
  // TODO
}
