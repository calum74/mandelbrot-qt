#pragma once

// TODO: Move some of these into AsyncRenderer.cpp when possible
#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "high_exponent_real.hpp"
#include "percentile.hpp"
#include "registry.hpp"
#include "rendering_sequence.hpp"
#include "view_coords.hpp"
#include "view_parameters.hpp"

#include <future>

namespace fractals {
void interpolate_region(Viewport &vp, int x0, int y0, int h);

class AsyncRenderer : public Renderer {

  const PointwiseFractal *current_fractal;
  view_coords coords;
  Registry &registry;
  std::future<void> current_calculation;
  std::atomic<bool> stop;
  std::shared_ptr<PointwiseCalculation> calculation;

  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

public:
  AsyncRenderer(const PointwiseFractal &fractal, Registry &registry);

  ~AsyncRenderer();

  void load(const view_parameters &params, Viewport &vp) override;

  void save(view_parameters &params) const override;

  void increase_iterations(Viewport &vp) override;

  void decrease_iterations(Viewport &vp) override;

  double get_average_iterations() const override;

  double get_average_skipped_iterations() const override;

  void discovered_depth(int points, double discovered_depth) override;

  void set_fractal(const fractals::PointwiseFractal &f) override;

  const fractals::PointwiseFractal &get_fractal() const override;

  view_coords initial_coords() const override;

  void stop_current_calculation();

  int center_x = 0, center_y = 0;

  // An array of all non-zero depths (?? Needed)

  struct depth_value {
    double depth;
    int x;
    int y;
    bool operator<(const depth_value &other) const {
      return depth < other.depth;
    }
  };

  std::vector<depth_value> depths;

  class my_rendering_sequence
      : public fractals::buffered_rendering_sequence<double> {

  public:
    my_rendering_sequence(const PointwiseCalculation &calculation,
                          const ColourMap &cm, Viewport &vp)
        : fractals::buffered_rendering_sequence<double>(vp.width, vp.height,
                                                        16),
          calculation(calculation), cm(cm), vp(vp) {}

    double min_depth = 0, max_depth = 0;
    int center_x = 0, center_y = 0;
    std::vector<depth_value> depths;

    void layer_complete(int stride) override {
      // Transfer and interpolate to the current viewport
      fractals::rendering_sequence seq(vp.width, vp.height, 16);
      seq.start_at_stride(stride);
      int x, y, s;
      bool c;
      while (seq.next(x, y, s, c) && stride == s) {
        double depth = output[x + y * vp.width];

        if (!std::isnan(depth)) {
          vp(x, y) = cm(depth);
          if (depth > 0) {
            depths.push_back({depth, x, y});
            if (depth > max_depth) {
              max_depth = depth;
            }
            if (depth < min_depth || min_depth == 0)
              min_depth = depth;
          }
#if 1 // Useful to be able to disable this for debugging
          if (stride > 1 && x > 0 && y > 0) {
            // Interpolate the region
            interpolate_region(vp, x - stride, y - stride, stride);
          }
#endif
        }
      }

      seq.start_at_stride(stride);
      long long total_x = 0, total_y = 0, total = 0;
      if (depths.size()) {
        auto discovered_depth =
            util::top_percentile(depths.begin(), depths.end(), 0.97)->depth;
        while (seq.next(x, y, s, c) && stride == s) {
          // Re-scan the points to find a center
          double depth = output[x + y * vp.width];
          if (depth == 0 || depth >= discovered_depth) {
            total_x += x;
            total_y += y;
            total++;
          }
        }
        center_x = total_x / total;
        center_y = total_y / total;
      }

      vp.region_updated(0, 0, vp.width, vp.height);
    }

    double get_point(int x, int y) override {
      // TODO: Avoid recalculating known points
      if (extra(vp(x, y)) == 0)
        return std::numeric_limits<double>::quiet_NaN();
      return calculation.calculate(x, y);
    }

  private:
    const PointwiseCalculation &calculation;
    const ColourMap &cm;
    Viewport &vp;
  };

  int threads = 4;
  int max_x = 0, max_y = 0;

  void calculate_region_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop);

  double view_min, view_max, view_percentile_max;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override;

  bool zoom(double r, int cx, int cy, Viewport &vp) override;

  view_coords get_coords() const override;

  double log_width() const override;

  int iterations() const override;

  void center(Viewport &vp) override;

  void zoom_in(Viewport &vp) override;

  int auto_remaining = 0;
  int auto_x, auto_y;

  void auto_step_continue(Viewport &vp) override;

  void auto_step(Viewport &vp) override;
  RGB grey = make_rgbx(100, 100, 100, 127);

  void remap_viewport(Viewport &vp, double dx, double dy, double r) const;

  void redraw(Viewport &vp) override;

  void set_aspect_ratio(int new_width, int new_height) override;

  bool set_coords(const view_coords &c, Viewport &vp) override;

  void scroll(int dx, int dy, Viewport &vp) override;

  bool automaticallyAdjustDepth = true;

  void enable_auto_depth(bool value) override;

  void set_threading(int threads) override;

  void get_depth_range(double &min, double &p, double &max) override;
};
} // namespace fractals