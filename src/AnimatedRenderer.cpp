#include "AnimatedRenderer.hpp"
#include "Renderer.hpp"
#include "registry.hpp"
#include <cassert>

using namespace std::literals::chrono_literals;

void register_fractals(fractals::Registry &r);

AnimatedRenderer::AnimatedRenderer()
    : colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()},
      renderer{fractals::make_renderer(*registry)} {

  register_fractals(*registry);
}

AnimatedRenderer::~AnimatedRenderer() { renderer.reset(); }

void AnimatedRenderer::calculate_async(fractals::Viewport &vp) {
  viewport = &vp;
  renderer->calculate_async(vp, *colourMap);
}

void AnimatedRenderer::renderFinishedBackgroundImage() {
  // if (!zooming)
  //    return;
  assert(viewport->size() == background_viewport.size());
  std::copy(background_viewport.begin(), background_viewport.end(),
            viewport->begin());
  viewport->updated();
}

void AnimatedRenderer::smoothZoomTo(int x, int y, bool lockCenter) {
  zooming = true;
  calculationFinished = false;
  zoomTimeout = false;

  computedImageData.resize(viewport->width * viewport->height);
  zoom_x = x;
  zoom_y = y;

  previousImageData.resize(viewport->width * viewport->height);
  std::copy(viewport->begin(), viewport->end(), previousImageData.begin());
  zoom_start = std::chrono::system_clock::now();
  // Add a 10% buffer to reduce stuttering
  zoom_duration = std::chrono::milliseconds(
      int(estimatedSecondsPerPixel * 1000 * viewport->width * viewport->height *
          1.10)); // Stupid stupid std::chrono

  // Stop the zoom duration getting too out of hand
  if (zoom_duration < 750ms)
    zoom_duration = 750ms;

  if (fixZoomSpeed)
    zoom_duration = fixZoomDuration; // Override for speed

  assert(computedImage.width() > 0);

  background_viewport.renderer = this;
  background_viewport.data = computedImageData.data();
  background_viewport.width = viewport->width;
  background_viewport.height = viewport->height;
  background_viewport.invalidateAllPixels();

  renderer->zoom(0.5, zoom_x, zoom_y, lockCenter, background_viewport);
  renderer->calculate_async(background_viewport, *colourMap);
}

void AnimatedRenderer::BackgroundViewport::updated() {}

void AnimatedRenderer::BackgroundViewport::finished(double width, int min_depth,
                                                    int max_depth, double avg,
                                                    double skipped,
                                                    double render_time) {
  // if (!widget->zooming)
  //   return;
  renderer->backgroundRenderFinished();
  renderer->viewport->finished(width, min_depth, max_depth, avg, skipped,
                               render_time);
}

void AnimatedRenderer::BackgroundViewport::discovered_depth(
    int points, double discovered_depth, double seconds_per_pixel) {
  if (renderer->renderer)
    renderer->renderer->discovered_depth(points, discovered_depth);
  renderer->estimatedSecondsPerPixel = seconds_per_pixel;
}

void AnimatedRenderer::backgroundRenderFinished() {
  calculationFinished = true;

  if (zoomTimeout) {
    renderFinishedBackgroundImage();
    zooming = false;
    beginNextAnimation();
  }
}

void AnimatedRenderer::beginNextAnimation() {
  if (!calculationFinished) {
    // Report on current calculation
    viewport->calculation_started(renderer->log_width(),
                                  renderer->iterations());
  }

  // We can't start a new calculation yet because we are within
  // the thread function of an existing calculation. So we signal that we need
  // to be called back
  viewport->schedule_next_calculation();
}

void AnimatedRenderer::timer() {
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
      viewport->calculation_started(renderer->log_width(),
                                    renderer->iterations());
    }
  } else {
    // Update the current view using the
    // The scaling ratio isn't actually linear !!
    // Project the current view into the frame
    auto zoom_ratio = std::pow(0.5, time_ratio);
    fractals::Viewport previousVp;
    previousVp.data = previousImageData.data();
    previousVp.width = viewport->width;
    previousVp.height = viewport->height;

    fractals::map_viewport(previousVp, *viewport, zoom_x * (1 - zoom_ratio),
                           zoom_y * (1 - zoom_ratio), zoom_ratio);
    viewport->updated();

    viewport->start_timer();
  }
}
