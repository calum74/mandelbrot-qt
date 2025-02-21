#include "AnimatedRenderer.hpp"
#include "Renderer.hpp"
#include "RenderingMetrics.hpp"
#include "registry.hpp"
#include "view_coords.hpp"
#include <cassert>

using namespace std::literals::chrono_literals;

void register_fractals(fractals::Registry &r);

fractals::AnimatedRenderer::AnimatedRenderer(fractals::Viewport &viewport)
    : viewport(viewport), colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()},
      renderer{fractals::make_renderer(*registry)}, background_viewport(*this) {

  register_fractals(*registry);
}

fractals::AnimatedRenderer::~AnimatedRenderer() { renderer.reset(); }

void fractals::AnimatedRenderer::calculate_async() {
  renderer->calculate_async(viewport, *colourMap);
}

void fractals::AnimatedRenderer::render_update_background_image() {
  // if (!zooming)
  //    return;
  assert(viewport.size() == background_viewport.size());
  for (int j = 0; j < viewport.height(); ++j)
    for (int i = 0; i < viewport.width(); ++i) {
      auto &from_pixel = background_viewport(i, j);
      auto &to_pixel = viewport(i, j);
      if (from_pixel.error < to_pixel.error) {
        to_pixel = from_pixel;
      }
    }
  viewport.updated();
}

void fractals::AnimatedRenderer::render_overwrite_background_image() {
  std::copy(background_viewport.begin(), background_viewport.end(),
            viewport.begin());
  viewport.updated();
}

void fractals::AnimatedRenderer::smooth_zoom_to(int x, int y, bool lockCenter) {
  zooming = true;
  calculationFinished = false;
  zoomTimeout = false;

  zoom_x = x;
  zoom_y = y;

  previousVp = viewport; // Copy everything by value

  zoom_start = std::chrono::system_clock::now();
  // Add a 10% buffer to reduce stuttering
  zoom_duration = std::chrono::milliseconds(
      int(estimatedSecondsPerPixel * 1000 * viewport.width() *
          viewport.height() * 1.10)); // Stupid stupid std::chrono

  // Stop the zoom duration getting too out of hand
  if (zoom_duration < fixZoomDuration)
    zoom_duration = fixZoomDuration;

  if (fixZoomSpeed)
    zoom_duration = fixZoomDuration; // Override for speed

  (fractals::Viewport &)background_viewport = viewport; // Copy everything over
  rendered_zoom_ratio = 1.0;
  calculated_points = 0;
  view_min = view_max = 0;

  renderer->zoom(0.5, zoom_x, zoom_y, lockCenter, background_viewport);
  renderer->calculate_async(background_viewport, *colourMap);
  viewport.start_timer();
}

void fractals::AnimatedRenderer::BackgroundViewport::updated() {
  if (renderer.zoomTimeout)
    renderer.render_update_background_image();
}

void fractals::AnimatedRenderer::BackgroundViewport::finished(
    const RenderingMetrics &metrics) {
  renderer.calculated_points = metrics.points_calculated;
  renderer.view_min = metrics.min_depth;
  renderer.view_max = metrics.max_depth;
  if (renderer.renderer)
    renderer.renderer->discovered_depth(metrics);
  renderer.estimatedSecondsPerPixel = metrics.seconds_per_point;

  if (metrics.fully_evaluated) {
    renderer.background_render_finished();
    renderer.viewport.finished(metrics);
  }
}

void fractals::AnimatedRenderer::background_render_finished() {
  calculationFinished = true;

  if (zoomTimeout) {
    render_update_background_image();
    zooming = false;
    begin_next_animation();
  }
}

void fractals::AnimatedRenderer::begin_next_animation() {
  if (!calculationFinished) {
    // Report on current calculation
    viewport.calculation_started(renderer->log_width(), renderer->iterations());
  }

  // If calculation is too slow, abort any animations that are in flight.
  if (calculated_points > 0 && view_min + 5 > view_max) {
    // Don't animate if we're in a low range
    // We're either in the middle of some black
    // or escaped.
    cancel_animations();
    return;
  }

  // We can't start a new calculation yet because we are within
  // the thread function of an existing calculation. So we signal that we need
  // to be called back
  viewport.schedule_next_calculation();
}

void fractals::AnimatedRenderer::timer() {
  if (!zooming)
    return;

  auto now = std::chrono::system_clock::now();
  double time_ratio =
      std::chrono::duration<double>(now - zoom_start) / zoom_duration;
  if (time_ratio >= 1) {
    zoomTimeout = true;
    // Maybe carry on zooming to the next frame
    if (calculationFinished || fixZoomSpeed) {
      // Copy the zoomed image exactly,
      // then superimpose whatever we calculated
      rendered_zoom_ratio = 0.5;

      fractals::map_viewport(
          previousVp, viewport, zoom_x * (1.0 - rendered_zoom_ratio),
          zoom_y * (1.0 - rendered_zoom_ratio), rendered_zoom_ratio);

      render_update_background_image();
      begin_next_animation();
    } else {
      // It's taking some time, so update the status bar
      viewport.calculation_started(renderer->log_width(),
                                   renderer->iterations());
    }
  } else {
    // Update the current view using the
    // The scaling ratio is exponential, not linear !!
    // Project the current view into the frame
    rendered_zoom_ratio = std::pow(0.5, time_ratio);

    fractals::map_viewport(
        previousVp, viewport, zoom_x * (1.0 - rendered_zoom_ratio),
        zoom_y * (1.0 - rendered_zoom_ratio), rendered_zoom_ratio);
    viewport.updated();

    viewport.start_timer();
  }
}

void fractals::AnimatedRenderer::cancel_animations() {
  viewport.stop_timer();
  current_animation = AnimatedRenderer::AnimationType::none;
  if (zooming) {
    if (zoomTimeout) {
      render_update_background_image();
    } else // Reset the coordinates back to what's shown
    {
      renderer->zoom(2.0 * rendered_zoom_ratio, zoom_x, zoom_y, false,
                     viewport);
      // Overwrite the viewport with the background image again
      // since this is more accurate than what's in the renderer
      // !! Refactor if this works

      fractals::map_viewport(
          previousVp, viewport, zoom_x * (1.0 - rendered_zoom_ratio),
          zoom_y * (1.0 - rendered_zoom_ratio), rendered_zoom_ratio);
    }
    zooming = false;
    // current_animation = AnimatedRenderer::AnimationType::none;
  }
}

void fractals::AnimatedRenderer::start_next_calculation() {
  switch (current_animation) {
  case AnimationType::autozoom:
    auto_navigate();
    break;
  case AnimationType::zoomtopoint:
    if (renderer->log_width() > zoomtopoint_limit)
      smooth_zoom_to(viewport.width() / 2, viewport.height() / 2, true);
    break;
  case AnimationType::zoomatcursor:
    smooth_zoom_to(move_x, move_y, false);
    break;
  default:
    break;
  }
}

void fractals::AnimatedRenderer::auto_navigate() {
  cancel_animations();
  int x, y;
  if (renderer->get_auto_zoom(x, y)) {
    current_animation = AnimationType::autozoom;
    smooth_zoom_to(x, y, false);
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
  auto c = renderer->get_coords();
  c.r = 2.0;
  c.max_iterations = 500;
  zoomtopoint_limit = renderer->log_width();
  renderer->set_coords(c, viewport);
}

void fractals::AnimatedRenderer::zoom_at_cursor() {
  if (zooming && !zoomTimeout) {
    cancel_animations();
    calculate_async();
    zooming = false;
  } else {
    cancel_animations();
    current_animation = AnimationType::zoomatcursor;
    smooth_zoom_to(move_x, move_y, false);
  }
}

void fractals::AnimatedRenderer::smooth_zoom_in() {
  if (zooming && !zoomTimeout) {
    cancel_animations();
    calculate_async();
    zooming = false;
  } else {
    cancel_animations();
    smooth_zoom_to(move_x, move_y, false);
  }
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

fractals::AnimatedRenderer::BackgroundViewport::BackgroundViewport(
    AnimatedRenderer &renderer)
    : renderer(renderer) {}

fractals::mapped_point
fractals::AnimatedRenderer::map_point(const view_coords &c) const {

  // Look at the zoom ratio
  // Use zoom_x, zoom_y and rendered_zoom_ratio

  auto original_coords = renderer->get_coords();

  if (zooming) {

    auto zoomed_coords =
        original_coords.zoom(2.0 * rendered_zoom_ratio, viewport.width(),
                             viewport.height(), zoom_x, zoom_y);

    return zoomed_coords.map_point(viewport.width(), viewport.height(), c);
  } else {
    return original_coords.map_point(viewport.width(), viewport.height(), c);
  }
}

void fractals::AnimatedRenderer::discovered_depth(
    const RenderingMetrics &metrics) {
  renderer->discovered_depth(metrics);

  if (!metrics.last_action_was_a_scroll && metrics.points_calculated > 1000 && metrics.min_depth>0) {
    colourMap->maybeUpdateRange(metrics.min_depth, metrics.max_depth);
  }
}

void fractals::AnimatedRenderer::enable_auto_gradient() {
  colourMap->enableAutoGradient();
}

void fractals::AnimatedRenderer::disable_auto_gradient() {
  colourMap->disableAutoGradient();

  if(is_animating())
    renderer->redraw(viewport);
}
