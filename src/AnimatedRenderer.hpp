#pragma once
#include "ColourMap.hpp"
#include "RGB.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "registry.hpp"

#include <atomic>
#include <chrono>

/*
Logic for creating animations.

Manages two additional pixel buffers for the previous image and the current
image being calculated.

*/
class AnimatedRenderer {
public:
  AnimatedRenderer(fractals::Viewport &viewport);
  ~AnimatedRenderer();

  void smoothZoomTo(int x, int y, bool lockCenter);

  void calculate_async(fractals::Viewport &output);

  void start_next_calculation();

  // Called when the timer has timed out
  void timer();

  void cancelAnimations();

  void set_cursor(int move_x, int move_y);
  void autoZoom();

public: // !! private
  // TODO: Make all this private
  std::unique_ptr<fractals::Registry> registry;

  // Note destruction order - renderer must be destroyed after viewport
  std::unique_ptr<fractals::Renderer> renderer;

  bool zooming = false;
  std::atomic<bool> calculationFinished = false;
  std::atomic<bool> zoomTimeout = false;
  std::chrono::time_point<std::chrono::system_clock> zoom_start;
  std::chrono::duration<double> zoom_duration;
  int zoom_x, zoom_y;
  double zoomtopoint_limit;

  enum class AnimationType {
    none,
    autozoom,
    startzoomtopoint,
    zoomtopoint,
    zoomatcursor
  } current_animation = AnimationType::none;

  double estimatedSecondsPerPixel = 0;
  std::vector<fractals::RGB> previousImagePixels, backgroundImagePixels;

  std::unique_ptr<fractals::ColourMap> colourMap;

  bool fixZoomSpeed = false;
  std::chrono::duration<double> fixZoomDuration;

  std::vector<fractals::RGB> previousImageData, computedImageData;

  struct BackgroundViewport : public fractals::Viewport {
    AnimatedRenderer *renderer;
    void updated() override;
    void finished(double width, int min_depth, int max_depth, double avg,
                  double skipped, double render_time) override;
    void discovered_depth(int points, double discovered_depth,
                          double time) override;
  } background_viewport;

  void renderFinishedBackgroundImage();
  void backgroundRenderFinished();
  void beginNextAnimation();

private:
  fractals::Viewport *viewport; // !! Use a reference
  int move_x = 0, move_y = 0;
};