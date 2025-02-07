#pragma once
#include "ColourMap.hpp"
#include "RGB.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "registry.hpp"

#include <atomic>
#include <chrono>

namespace fractals {
class mapped_point;
/*
Logic for creating animations.

Manages two additional pixel buffers for the previous image and the current
image being calculated.

*/
class AnimatedRenderer {
public:
  AnimatedRenderer(fractals::Viewport &viewport);
  ~AnimatedRenderer();

  void smooth_zoom_to(int x, int y, bool lockCenter);

  void calculate_async();

  void start_next_calculation();

  // Called when the timer has timed out
  void timer();

  void cancel_animations();

  void set_cursor(int move_x, int move_y);
  void auto_navigate();
  void animate_to_here();
  void zoom_at_cursor();
  void smooth_zoom_in();
  void set_speed_estimate(double seconds_per_pixel);

  // fixedSpeed means that we don't wait for rendering to complete
  void set_animation_speed(std::chrono::duration<double> speed,
                           bool fixedSpeed);

  bool is_animating() const;

public: // !! Ideally private
  std::unique_ptr<fractals::Registry> registry;
  std::unique_ptr<fractals::Renderer> renderer;
  std::unique_ptr<fractals::ColourMap> colourMap;

  enum class AnimationType {
    none,
    autozoom,
    startzoomtopoint,
    zoomtopoint,
    zoomatcursor
  } current_animation = AnimationType::none;

  mapped_point map_point(const view_coords &c) const;

private:
  bool zooming = false;
  std::atomic<bool> calculationFinished = false;
  std::atomic<bool> zoomTimeout = false;
  std::chrono::time_point<std::chrono::system_clock> zoom_start;
  std::chrono::duration<double> zoom_duration;
  int zoom_x, zoom_y;
  double zoomtopoint_limit;
  double rendered_zoom_ratio;
  int calculated_points;
  int view_min, view_max;

  double estimatedSecondsPerPixel = 0;
  std::vector<fractals::RGB> previousImagePixels, backgroundImagePixels;

  bool fixZoomSpeed = false;
  std::chrono::duration<double> fixZoomDuration;

  std::vector<fractals::RGB> previousImageData, computedImageData;

  struct BackgroundViewport : public fractals::Viewport {
    BackgroundViewport(AnimatedRenderer &);
    AnimatedRenderer &renderer;
    void updated() override;
    void finished(const RenderingMetrics &metrics) override;
  } background_viewport;

  void render_update_background_image();
  void render_overwrite_background_image();
  void background_render_finished();
  void begin_next_animation();

  fractals::Viewport &viewport;
  int move_x = 0, move_y = 0;
};
} // namespace fractals
