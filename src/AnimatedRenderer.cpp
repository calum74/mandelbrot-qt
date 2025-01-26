#include "AnimatedRenderer.hpp"
#include "Renderer.hpp"
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
  for (int i = 0; i < viewport.size(); ++i) {
    auto from_pixel = background_viewport.data[i];
    auto &to_pixel = viewport.data[i];
    if (extra(from_pixel) < extra(to_pixel))
      to_pixel = from_pixel;
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

  computedImageData.resize(viewport.width * viewport.height);
  zoom_x = x;
  zoom_y = y;

  previousImageData.resize(viewport.width * viewport.height);
  std::copy(viewport.begin(), viewport.end(), previousImageData.begin());
  std::copy(viewport.begin(), viewport.end(), computedImageData.begin());

  zoom_start = std::chrono::system_clock::now();
  // Add a 10% buffer to reduce stuttering
  zoom_duration = std::chrono::milliseconds(
      int(estimatedSecondsPerPixel * 1000 * viewport.width * viewport.height *
          1.10)); // Stupid stupid std::chrono

  // Stop the zoom duration getting too out of hand
  if (zoom_duration < fixZoomDuration)
    zoom_duration = fixZoomDuration;

  if (fixZoomSpeed)
    zoom_duration = fixZoomDuration; // Override for speed

  background_viewport.data = computedImageData.data();
  background_viewport.width = viewport.width;
  background_viewport.height = viewport.height;
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
  // if (!widget->zooming)
  //   return;
  renderer.background_render_finished();
  renderer.viewport.finished(metrics);
  // if (max_depth - min_depth < 5)
  //  renderer.cancel_animations();
}

void fractals::AnimatedRenderer::BackgroundViewport::discovered_depth(
    int points, double discovered_depth, double seconds_per_pixel, int view_min,
    int view_max, int total_points) {
  renderer.calculated_points = total_points;
  renderer.view_min = view_min;
  renderer.view_max = view_max;
  std::cout << "discovered_depth min=" << view_min << ", max=" << view_max
            << "\n";
  if (renderer.renderer)
    renderer.renderer->discovered_depth(points, discovered_depth, view_min,
                                        view_max, total_points);
  renderer.estimatedSecondsPerPixel = seconds_per_pixel;
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

  if (calculated_points == 0)
    std::cout << "No points\n";
  // If calculation is too slow, abort any animations that are in flight.
  if (calculated_points > 0 && view_min + 5 > view_max) {
    std::cout << "Low range\n";
    std::cout << calculated_points << " points, " << view_min << "-" << view_max
              << std::endl;
    // Don't animate if we're in a low range
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
      fractals::Viewport previousVp;
      previousVp.data = previousImageData.data();
      previousVp.width = viewport.width;
      previousVp.height = viewport.height;

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
    // The scaling ratio isn't actually linear !!
    // Project the current view into the frame
    rendered_zoom_ratio = std::pow(0.5, time_ratio);
    fractals::Viewport previousVp;
    previousVp.data = previousImageData.data();
    previousVp.width = viewport.width;
    previousVp.height = viewport.height;

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
      fractals::Viewport previousVp;
      previousVp.data = previousImageData.data();
      previousVp.width = viewport.width;
      previousVp.height = viewport.height;

      fractals::map_viewport(
          previousVp, viewport, zoom_x * (1.0 - rendered_zoom_ratio),
          zoom_y * (1.0 - rendered_zoom_ratio), rendered_zoom_ratio);
    }
    zooming = false;
  }
}

void fractals::AnimatedRenderer::start_next_calculation() {
  // !! switch statement
  switch (current_animation) {
  case AnimationType::autozoom:
    auto_navigate();
    break;
  case AnimatedRenderer::AnimationType::zoomtopoint:
    if (renderer->log_width() > zoomtopoint_limit)
      smooth_zoom_to(viewport.width / 2, viewport.height / 2, true);
    break;
  case AnimatedRenderer::AnimationType::zoomatcursor:
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

fractals::AnimatedRenderer::BackgroundViewport::BackgroundViewport(
    AnimatedRenderer &renderer)
    : renderer(renderer) {}