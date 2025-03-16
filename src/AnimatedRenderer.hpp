#pragma once
#include "ColourMap.hpp"
#include "rgb.hpp"
#include "registry.hpp"
#include "view_listener.hpp"
#include "view_animation.hpp"

#include <atomic>
#include <chrono>
#include <optional>

namespace fractals {
class mapped_point;

/*
Logic for creating animations.

Manages two additional pixel buffers for the previous image and the current
image being calculated.

*/
class AnimatedRenderer : view_listener {
public:
  AnimatedRenderer(fractals::view_listener &listener2);
  ~AnimatedRenderer();

  void smooth_zoom_to(int x, int y, bool lockCenter, std::optional<std::chrono::duration<double>> duration);

  void calculate_async();

  void cancel_animations();

  void set_cursor(int move_x, int move_y);
  void auto_navigate();
  void animate_to_here();
  void zoom_at_cursor();
  void smooth_zoom_in();
  void discovered_depth(const calculation_metrics &metrics);

  // fixedSpeed means that we don't wait for rendering to complete
  void set_animation_speed(std::chrono::duration<double> speed,
                           bool fixedSpeed);

  bool is_animating() const;

  void enable_auto_gradient();
  void disable_auto_gradient();

  double ln_r() const;
  int iterations() const;
  void scroll(int x, int y);
  void resize(int w, int h);
  void zoom(double f, int x, int y, bool fix_center);
  void update_iterations(const calculation_metrics &);
  void increase_iterations();
  void decrease_iterations();
  void load(const view_parameters &params);
  void save(view_parameters &params) const;
  void set_coords(const view_coords&);
  std::string fractal_family() const;
  std::string fractal_name() const;
  view_coords initial_coords() const;
  void set_fractal(const fractal&, bool reset_coords);
  void values_changed() override;
  void enable_auto_depth(bool enabled);
  void set_threading(int);
  void get_depth_range(double&, double&, double&) const;

public: // !! Ideally private
  std::unique_ptr<fractals::Registry> registry;
  std::unique_ptr<fractals::ColourMap> colourMap;

  mapped_point map_point(const view_coords &c) const;

private:


  void background_render_finished();
  void begin_next_animation();

  void calculation_started(radius r, int max_iterations) override;
  void calculation_finished(const calculation_metrics &) override;
  void animation_finished(const calculation_metrics &) override;

  fractals::view_listener &listener;
  int move_x = 0, move_y = 0;

  public:
  fractals::view_animation view;
};
} // namespace fractals
