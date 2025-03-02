#include "AnimatedRenderer.hpp"
#include "Renderer.hpp"
#include "calculation_metrics.hpp"
#include "mandelbrot.hpp"
#include "registry.hpp"
#include "shader_parameters.hpp"
#include "view_coords.hpp"
#include "view_parameters.hpp"
#include <cassert>

using namespace std::literals::chrono_literals;

void register_fractals(fractals::Registry &r);

fractals::AnimatedRenderer::AnimatedRenderer(fractals::view_listener &listener)
    : listener(listener), colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()} {

  register_fractals(*registry);
  single_zoom_duration = 250ms; // 50ms;
  continuous_zoom_duration = 750ms;

  view.set_threading(4);

  view.set_fractal(mandelbrot_fractal, true, false);
  view.set_listener(this);
}

fractals::AnimatedRenderer::~AnimatedRenderer() {}

void fractals::AnimatedRenderer::calculate_async() { view.start_calculating(); }

void fractals::AnimatedRenderer::calculation_finished(const calculation_metrics &metrics) {

  // Update the iterations etc.
  std::cout << "  Finished - setting new range to " << metrics.discovered_depth << std::endl;

  if(metrics.discovered_depth>0 && metrics.discovered_depth > 250)
  {
    view.set_max_iterations(metrics.discovered_depth * 2);
    colourMap->maybeUpdateRange(metrics.min_depth, metrics.discovered_depth);
  }

  listener.calculation_finished(metrics);
}

void fractals::AnimatedRenderer::animation_finished(
    const calculation_metrics &) {
  std::cout << "Animation completed\n";
}

void fractals::AnimatedRenderer::values_changed() {
  listener.values_changed();
}

void fractals::AnimatedRenderer::smooth_zoom_to(
    int x, int y, bool lockCenter,
    std::optional<std::chrono::duration<double>> requested_duration) {
  zooming = true;
  calculationFinished = false;
  zoomTimeout = false;

  zoom_x = x;
  zoom_y = y;

  zoom_start = std::chrono::system_clock::now();
  // Add a 10% buffer to reduce stuttering
  zoom_duration = std::chrono::milliseconds(
      int(estimatedSecondsPerPixel * 1000 * view.width() *
          view.height() * 1.10)); // Stupid stupid std::chrono

  // Stop the zoom duration getting too out of hand
  if (zoom_duration < fixZoomDuration)
    zoom_duration = fixZoomDuration;

  if (fixZoomSpeed)
    zoom_duration = fixZoomDuration; // Override for speed

  if (requested_duration && fixZoomSpeed) {
    zoom_duration = *requested_duration;
  }

  rendered_zoom_ratio = 1.0;
  calculated_points = 0;
  view_min = view_max = 0;

  if (lockCenter)
    view.animate_to_center(zoom_duration, !fixZoomSpeed);
  else
    view.animate_to(x, y, zoom_duration, !fixZoomSpeed);
}

void fractals::AnimatedRenderer::begin_next_animation() {
  if (!calculationFinished) {
    // Report on current calculation
    listener.calculation_started(view.get_coords().ln_r(),
                                 view.get_coords().max_iterations);
  }

  // If calculation is too slow, abort any animations that are in flight.
  if (calculated_points > 0 && view_min + 5 > view_max) {
    // Don't animate if we're in a low range
    // We're either in the middle of some black
    // or escaped.
    cancel_animations();
    return;
  }

  // TODO
}

void fractals::AnimatedRenderer::cancel_animations() {
  view.stop_current_animation_and_set_as_current();
}

void fractals::AnimatedRenderer::start_next_calculation() {
  switch (current_animation) {
  case AnimationType::autozoom:
    auto_navigate();
    break;
  case AnimationType::zoomtopoint:
    if (view.get_coords().ln_r() > zoomtopoint_limit)
      smooth_zoom_to(view.width() / 2, view.height() / 2, true, {});
    break;
  case AnimationType::zoomatcursor:
    smooth_zoom_to(move_x, move_y, false, continuous_zoom_duration);
    break;
  default:
    break;
  }
}

void fractals::AnimatedRenderer::auto_navigate() {
  cancel_animations();
  int x, y;
  if (view.get_auto_zoom(x, y)) {
    current_animation = AnimationType::autozoom;
    smooth_zoom_to(x, y, false, {});
  } else {
    std::cout << "Autozoom continue failed\n";
  }
}

void fractals::AnimatedRenderer::set_cursor(int x, int y) {
  move_x = x;
  move_y = y;
}

void fractals::AnimatedRenderer::animate_to_here() {
  cancel_animations();
  current_animation = AnimationType::startzoomtopoint;
  auto c = view.get_coords();
  c.r = 2.0;
  c.max_iterations = 500;
  zoomtopoint_limit = view.get_coords().ln_r();
  view.set_coords(c, true);
}

void fractals::AnimatedRenderer::zoom_at_cursor() {
  if (zooming && !zoomTimeout) {
    cancel_animations();
    calculate_async();
    zooming = false;
  } else {
    cancel_animations();
    current_animation = AnimationType::zoomatcursor;
    smooth_zoom_to(move_x, move_y, false, continuous_zoom_duration);
  }
}

void fractals::AnimatedRenderer::smooth_zoom_in() {
  smooth_zoom_to(move_x, move_y, false, single_zoom_duration);
}

void fractals::AnimatedRenderer::set_speed_estimate(double secondsPerPixel) {
  estimatedSecondsPerPixel = secondsPerPixel;
}

void fractals::AnimatedRenderer::set_animation_speed(
    std::chrono::duration<double> speed, bool fixedSpeed) {
  fixZoomSpeed = fixedSpeed;
  fixZoomDuration = speed;
}

bool fractals::AnimatedRenderer::is_animating() const { return zooming; }

fractals::mapped_point
fractals::AnimatedRenderer::map_point(const view_coords &c) const {

  // Look at the zoom ratio
  // Use zoom_x, zoom_y and rendered_zoom_ratio

  auto original_coords = view.get_coords();

  if (zooming) {

    auto zoomed_coords =
        original_coords.zoom(2.0 * rendered_zoom_ratio, view.width(),
                             view.height(), zoom_x, zoom_y);

    return zoomed_coords.map_point(view.width(), view.height(), c);
  } else {
    return original_coords.map_point(view.width(), view.height(), c);
  }
}

void fractals::AnimatedRenderer::discovered_depth(
    const calculation_metrics &metrics) {
  view.update_iterations(metrics);

  if (!metrics.last_action_was_a_scroll && metrics.points_calculated > 1000 &&
      metrics.min_depth > 0) {
    colourMap->maybeUpdateRange(metrics.min_depth, metrics.max_depth);
  }
}

void fractals::AnimatedRenderer::enable_auto_gradient() {
  fractals::shader_parameters params;
  colourMap->getParameters(params);
  params.auto_gradient = true;
  colourMap->setParameters(params);
}

void fractals::AnimatedRenderer::disable_auto_gradient() {
  fractals::shader_parameters params;
  colourMap->getParameters(params);
  params.auto_gradient = false;
  colourMap->setParameters(params);
}

void fractals::AnimatedRenderer::update_iterations(
    const calculation_metrics &metrics) {
  view.update_iterations(metrics);
}

double fractals::AnimatedRenderer::ln_r() const {
  return view.get_coords().ln_r();
}

int fractals::AnimatedRenderer::iterations() const {
  return view.get_coords().max_iterations;
}

void fractals::AnimatedRenderer::scroll(int x, int y) {
  view.scroll(x, y);
}

void fractals::AnimatedRenderer::resize(int w, int h) { view.set_size(w, h); }

void fractals::AnimatedRenderer::zoom(double f, int x, int y, bool fix_center) {
  view.zoom(x, y, f);
}

void fractals::AnimatedRenderer::increase_iterations() {
  view.increase_iterations();
}

void fractals::AnimatedRenderer::decrease_iterations() {
  view.decrease_iterations();
}

void fractals::AnimatedRenderer::load(const view_parameters &params) {
  colourMap->load(params);
  // Set the fractal

  auto new_fractal = registry->lookup(params.algorithm);

  if (new_fractal) {
    view.set_fractal(*new_fractal, false, false);
  }

  view_coords new_coords = params;
  std::cout << "1: Setting the new coords to " << new_coords.ln_r() << std::endl;
  view.set_coords(new_coords, true);
  std::cout << "2: Setting the new coords to " << new_coords.ln_r() << std::endl;
}

void fractals::AnimatedRenderer::save(view_parameters &params) const {
  view.get_coords().write(params);
  params.algorithm = view.get_fractal_name();
  colourMap->save(params);
}

void fractals::AnimatedRenderer::set_coords(const view_coords &coords) {
  view.set_coords(coords, true);
}

std::string fractals::AnimatedRenderer::fractal_family() const {
  return view.get_fractal_family();
}

std::string fractals::AnimatedRenderer::fractal_name() const {
  return view.get_fractal_name();
}

fractals::view_coords fractals::AnimatedRenderer::initial_coords() const {
  return view.initial_coords();
}

void fractals::AnimatedRenderer::set_fractal(const fractal &f, bool init_coords) {
  view.set_fractal(f, init_coords, true);
}

void fractals::AnimatedRenderer::enable_auto_depth(bool enabled) {

  // renderer->enable_auto_depth(enabled);
}

void fractals::AnimatedRenderer::set_threading(int n) { view.set_threading(n); }

void fractals::AnimatedRenderer::get_depth_range(double &a, double &b,
                                                 double &c) const {
  auto &metrics = view.get_metrics();
  a = metrics.min_depth;
  b = metrics.discovered_depth;
  c = metrics.max_depth;
}

void fractals::AnimatedRenderer::calculation_started(radius r, int max_iterations)
{
  listener.calculation_started(r, max_iterations);
}