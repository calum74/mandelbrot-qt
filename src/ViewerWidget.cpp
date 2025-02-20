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
    : QWidget{parent}, viewport(*this), renderer(viewport) {
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

  // viewport.init(image.width(), image.height(), (fractals::RGB
  // *)image.bits());

  renderer.calculate_async();
  // calculateFlagLocations();
}

void ViewerWidget::draw() {
  pending_redraw = 0;
  QPainter painter(this);

  std::uint32_t *image_data = (std::uint32_t *)image.bits();
  for (int i = 0; i < viewport.size(); ++i)
    image_data[i] = 0xff000000 | viewport[i].colour;

  painter.drawImage(this->rect(), image);

  std::lock_guard<std::mutex> lock(bookmarksMutex);
  if (!flagsToDraw.empty()) {
    // QIcon::ThemeIcon::EditUndo
    // QIcon::ThemeIcon::NetworkWireless
    // QIcon::ThemeIcon::ZoomIn
    // QIcon::ThemeIcon::GoUp
    // QIcon::ThemeIcon::GoDown

    QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon::CameraPhoto);

    for (auto &p : flagsToDraw) {
      icon.paint(&painter, p.x, p.y, p.size, p.size);
    }
  }
}

constexpr fractals::Viewport::pixel grey = {fractals::make_rgb(100, 100, 100),
                                            127};

void ViewerWidget::doResize(int w, int h) {
  w *= imageScale;
  h *= imageScale;

  // Should stop the current calculation
  renderer.renderer->set_aspect_ratio(w, h);

  viewport.init(w, h);
  image = QImage(w, h, QImage::Format_RGB32);
  std::fill(viewport.begin(), viewport.end(), grey);
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
  calculateFlagLocations();
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
    widget.renderer.renderer->discovered_depth(metrics);
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
  renderer.cancel_animations();
  renderer.colourMap->randomize();
  renderer.renderer->redraw(viewport);
  calculate();
}

void ViewerWidget::resetCurrentFractal() {
  renderer.cancel_animations();
  renderer.renderer->set_coords(renderer.renderer->initial_coords(), viewport);
  renderer.colourMap->resetGradient();
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
  renderer.cancel_animations();
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

void ViewerWidget::showBookmarks(const fractals::view_parameters *params,
                                 int size) {
  std::vector<BookmarkToDraw> newBookmarks;
  for (int i = 0; i < size; ++i) {
    newBookmarks.push_back({params[i].algorithm, params[i]});
  }
  bookmarksToDraw = std::move(newBookmarks);
  doUpdate();
}

void ViewerWidget::hideBookmarks() {
  bookmarksToDraw.clear();
  doUpdate();
}

void ViewerWidget::calculateFlagLocations() {
  std::vector<flag_location> newFlags;
  auto name = renderer.renderer->get_fractal_name();

  // !! This might not be threadsafe
  for (auto &bm : bookmarksToDraw) {
    if (bm.algorithm == name) {
      auto p = renderer.map_point(bm.coords);
      if (p.x >= 0 && p.x < viewport.width() &&
          p.y >= 0 & p.y < viewport.height()) {
        int size = p.log_distance;
        if (size > 0) {
          size = 40 - size;
          if (size < 15)
            size = 15;
          if (size > 40)
            size = 40;
          newFlags.push_back({int(p.x), int(p.y), size});
        }
      }
    }
  }

  std::lock_guard<std::mutex> lock(bookmarksMutex);
  flagsToDraw = std::move(newFlags);
}