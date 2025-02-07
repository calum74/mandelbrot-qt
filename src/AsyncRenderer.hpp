#pragma once

#include "Renderer.hpp"
#include "RenderingMetrics.hpp"
#include "fractal.hpp"
#include "rendering_sequence.hpp"
#include "view_coords.hpp"

#include <future>

namespace fractals {

// This is an algorithm that fills in a region close to a point
// with that pixel value. However, it takes into consideration if a pixel has
// already been calculated, and decides if the interpolated value should
// overwrite each pixel or not. Each pixel has a concept of an "error" to decide
// when it should get updated. This leads to interesting visual effects.
void interpolate_region(Viewport &vp, int cx, int cy, int x0, int y0, int x1,
                        int y1);
bool maybe_fill_region(Viewport &vp, int x0, int y0, int x1, int y1);

class AsyncRenderer : public Renderer {

public:
  AsyncRenderer(const PointwiseFractal &fractal, Registry &registry);
  ~AsyncRenderer();

  void load(const view_parameters &params, Viewport &vp) override;
  void save(view_parameters &params) const override;
  void increase_iterations(Viewport &vp) override;
  void decrease_iterations(Viewport &vp) override;
  double get_average_iterations() const override;
  double get_average_skipped_iterations() const override;
  void discovered_depth(const RenderingMetrics &metrics) override;
  void set_fractal(const fractals::PointwiseFractal &f) override;
  const char *get_fractal_name() const override;
  const char *get_fractal_family() const override;
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
  std::pair<int, int> map_point(const Viewport &vp,
                                const view_coords &c) const override;

  // An array of all non-zero depths (?? Needed)

  struct depth_value {
    double depth;
    int x;
    int y;
    bool operator<(const depth_value &other) const {
      return depth < other.depth;
    }
  };

  class my_rendering_sequence
      : public fractals::buffered_rendering_sequence<double> {

  public:
    my_rendering_sequence(const PointwiseCalculation &calculation,
                          const ColourMap &cm, Viewport &vp,
                          std::vector<depth_value> &depths);

    std::atomic<double> min_depth = 0, max_depth = 0;
    std::vector<depth_value> &depths;
    std::atomic<std::uint64_t> calculated_pixels = 0;

    void layer_complete(int stride) override;

    double get_point(int x, int y) override;

  private:
    const PointwiseCalculation &calculation;
    const ColourMap &cm;
    Viewport &vp;
  };

  void stop_current_calculation();

  void calculate_region_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop);

private:
  std::shared_ptr<PointwiseCalculationFactory> current_fractal;
  view_coords coords;
  Registry &registry;
  std::future<void> current_calculation;
  std::atomic<bool> stop;
  std::shared_ptr<PointwiseCalculation> calculation;

  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

  std::vector<depth_value> depths; // ?? Do we want to keep this around

  RenderingMetrics metrics;

  int threads = 4;
  int max_x = 0, max_y = 0;
  bool automaticallyAdjustDepth = true;
};
} // namespace fractals