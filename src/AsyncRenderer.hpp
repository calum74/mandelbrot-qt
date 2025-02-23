#pragma once

#include "Renderer.hpp"
#include "RenderingMetrics.hpp"
#include "calculation_pixmap.hpp"
#include "fractal.hpp"
#include "mandelbrot_fwd.hpp"
#include "view_coords.hpp"

#include <future>

namespace fractals {

void interpolate_region(Viewport &vp, int cx, int cy, int x0, int y0, int x1,
                        int y1);
bool maybe_fill_region(Viewport &vp, int x0, int y0, int x1, int y1);

class AsyncRenderer : public Renderer {

public:
  AsyncRenderer(const fractal &fractal, Registry &registry);
  ~AsyncRenderer();

  void load(const view_parameters &params, Viewport &vp) override;
  void save(view_parameters &params) const override;
  void increase_iterations(Viewport &vp) override;
  void decrease_iterations(Viewport &vp) override;
  double get_average_iterations() const override;
  double get_average_skipped_iterations() const override;
  void discovered_depth(const RenderingMetrics &metrics) override;
  void set_fractal(const fractals::fractal &f) override;
  std::string get_fractal_name() const override;
  std::string get_fractal_family() const override;
  view_coords initial_coords() const override;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override;
  bool zoom(double r, int cx, int cy, bool lockCenter, Viewport &vp) override;
  view_coords get_coords() const override;
  double log_width() const override;
  int iterations() const override;
  void center(Viewport &vp) override;
  void zoom_in(Viewport &vp) override;
  void remap_viewport(Viewport &vp, double dx, double dy, double r) const;
  void redraw(Viewport &vp) override;
  void set_aspect_ratio(int new_width, int new_height) override;
  bool set_coords(const view_coords &c, Viewport &vp) override;
  void scroll(int dx, int dy, Viewport &vp) override;
  void enable_auto_depth(bool value) override;
  void set_threading(int threads) override;
  void get_depth_range(double &min, double &p, double &max) override;
  bool get_auto_zoom(int &x, int &y) override;
  mapped_point map_point(const Viewport &vp,
                         const view_coords &c) const override;

  class my_rendering_sequence : public fractals::calculation_pixmap {
  public:
    my_rendering_sequence(
        fractal_calculation &calculation,
        const ColourMap &cm, Viewport &vp);

    void layer_complete(int stride) override;

  private:
    const ColourMap &cm;
    Viewport &vp;
  };

  void stop_current_calculation();

  void calculate_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop);

private:
  std::shared_ptr<fractal_calculation_factory> current_fractal;
  view_coords coords;
  Registry &registry;
  std::future<void> current_calculation;
  std::atomic<bool> stop;
  std::shared_ptr<fractal_calculation> calculation;
  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

  RenderingMetrics metrics;

  int threads = 4;
  int max_x = 0, max_y = 0;
  bool automaticallyAdjustDepth = true;
};
} // namespace fractals
