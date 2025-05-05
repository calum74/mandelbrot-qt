#include "AnimatedRenderer.hpp"
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
    : listener(listener), colourMap{fractals::make_shader()},
      registry{fractals::make_registry()}, view(*this) {

  register_fractals(*registry);

  view.set_threading(4);

  view.set_fractal(mandelbrot_fractal, true, false);
}

fractals::AnimatedRenderer::~AnimatedRenderer() {}

void fractals::AnimatedRenderer::calculate_async() { view.start_calculating(); }

void fractals::AnimatedRenderer::calculation_finished(
    const calculation_metrics &metrics) {

  // Update the iterations etc.
  if (metrics.discovered_depth > 0 && metrics.discovered_depth > 250) {
    view.set_max_iterations(metrics.discovered_depth * 2);
    colourMap->maybeUpdateRange(metrics.min_depth, metrics.discovered_depth);
  }

  listener.calculation_finished(metrics);
}

void fractals::AnimatedRenderer::animation_finished(
    const calculation_metrics &) {
}

void fractals::AnimatedRenderer::values_changed() { listener.values_changed(); }

void fractals::AnimatedRenderer::smooth_zoom_to(
    int x, int y, bool lockCenter,
    std::optional<std::chrono::duration<double>> requested_duration) {
}

void fractals::AnimatedRenderer::cancel_animations() {
  view.stop_current_animation_and_set_as_current();
}

void fractals::AnimatedRenderer::set_cursor(int x, int y) {
  move_x = x;
  move_y = y;
  view.mouse_at(x, y);
}

void fractals::AnimatedRenderer::animate_to_here() {
  view.animate_to_current_position();
}

void fractals::AnimatedRenderer::zoom_at_cursor() {
  view.navigate_at_cursor(move_x, move_y);
}

void fractals::AnimatedRenderer::smooth_zoom_in() {
  view.smooth_zoom_at_cursor(move_x, move_y);
}

void fractals::AnimatedRenderer::set_animation_speed(
    std::chrono::duration<double> speed, bool fixedSpeed) {
  view.wait_for_completion = !fixedSpeed;
  view.animate_step_duration = speed;
}

bool fractals::AnimatedRenderer::is_animating() const {
  return view.is_animating();
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

numbers::radius fractals::AnimatedRenderer::radius() const {
  return view.get_coords().radius();
}

int fractals::AnimatedRenderer::iterations() const {
  return view.get_coords().max_iterations;
}

void fractals::AnimatedRenderer::scroll(int x, int y) { view.scroll(x, y); }

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
  view.set_coords(new_coords, true);
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

void fractals::AnimatedRenderer::set_fractal(const fractal &f,
                                             bool init_coords) {
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

void fractals::AnimatedRenderer::calculation_started(numbers::radius r,
                                                     int max_iterations) {
  listener.calculation_started(r, max_iterations);
}

void fractals::AnimatedRenderer::auto_navigate() {
  view.navigate_randomly();
}

bool fractals::AnimatedRenderer::fully_calculated() const { 
  return view.fully_calculated();
}

void fractals::AnimatedRenderer::get_orbit(int x, int y, displayed_orbit&orbit) const
{
  std::atomic<bool> stop;
  view.get_orbit(x, y, orbit, stop);
}
