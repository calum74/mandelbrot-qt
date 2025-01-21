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

  double get_average_skipped_iterations() const override {
    return calculation->average_skipped();
  }

  void discovered_depth(int points, double discovered_depth) override {
    if (points > 1000)                              // Fudge factor
      coords.max_iterations = discovered_depth * 2; // Fudge factor
  }

  void set_fractal(const fractals::PointwiseFractal &f) override {
    stop_current_calculation();
    current_fractal = &f;
  }

  const fractals::PointwiseFractal &get_fractal() const override {
    return *current_fractal;
  }

  view_coords initial_coords() const override {
    return current_fractal->initial_coords();
  }

  void stop_current_calculation() {
    stop = true;
    if (current_calculation.valid())
      current_calculation.get();
    stop = false;
  }

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
                                  std::atomic<bool> &stop) {

    my_rendering_sequence seq(*calculation, cm, vp);
    seq.calculate(threads, stop);
    view_min = seq.min_depth;
    view_max = seq.max_depth;
    center_x = seq.center_x;
    center_y = seq.center_y;

    depths = std::move(seq.depths);
  }

  double view_min, view_max, view_percentile_max;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override {
    stop_current_calculation();
    depths.clear();

    stop = false;

    current_calculation = std::async([&]() {
      calculation =
          current_fractal->create(coords, view.width, view.height, stop);

      t0 = std::chrono::high_resolution_clock::now();
      view_min = 0;
      view_max = 0;
      view_percentile_max = 0;
      calculate_region_in_thread(view, cm, stop);
      if (!stop) {
        view.region_updated(0, 0, view.width, view.height);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> d = t1 - t0;

        view.finished(log_width(), view_min, view_max,
                      calculation->average_iterations(),
                      calculation->average_skipped(), d.count());
      }

      if (automaticallyAdjustDepth && depths.begin() < depths.end()) {
        auto discovered_depth =
            util::top_percentile(depths.begin(), depths.end(), 0.999)->depth;
        view_percentile_max = discovered_depth;
        view.discovered_depth(std::distance(depths.begin(), depths.end()),
                              discovered_depth);
      }
    });
  }

  bool zoom(double r, int cx, int cy, Viewport &vp) override {
    stop_current_calculation();

    auto new_coords = coords.zoom(r, vp.width, vp.height, cx, cy);

    if (!current_fractal->valid_for(new_coords)) {
      return false;
    }

    coords = new_coords;

    remap_viewport(vp, cx * (1 - r), cy * (1 - r), r);

    vp.region_updated(0, 0, vp.width, vp.height);
    return true;
  }

  view_coords get_coords() const override { return coords; }

  double log_width() const override {
    return fractals::log(convert<high_exponent_real<>>(coords.r));
  }

  int iterations() const override { return coords.max_iterations; }

  void center(Viewport &vp) override {
    if (max_x > 0 && max_y > 0)
      scroll(max_x - vp.width / 2, max_y - vp.height / 2, vp);
  }

  void zoom_in(Viewport &vp) override {
    zoom(0.5, vp.width / 2, vp.height / 2, vp);
  }

  int auto_remaining = 0;
  int auto_x, auto_y;

  void auto_step_continue(Viewport &vp) override {
    stop_current_calculation();

    if (auto_remaining > 0) {
      auto_remaining--;
      zoom(0.75, auto_x, auto_y, vp);
      return;
    } else {
      auto_step(vp);
    }
  }

  void auto_step(Viewport &vp) override {
    stop_current_calculation();

    /*
        if (center_x > 0 && center_y > 0) {
          auto_x =
          zoom(0.85, center_x, center_y, vp);
          return;
        }
    */
    if (!depths.empty()) {

      auto p = util::top_percentile(depths.begin(), depths.end(), 0.9999);
      auto_x = p->x;
      auto_y = p->y;
      auto_remaining = 10;
      auto_step_continue(vp);
    }
  }

  RGB grey = make_rgbx(100, 100, 100, 127);

  void remap_viewport(Viewport &vp, double dx, double dy, double r) const {

    /*
      This is interesting so pay attention.
      Each pixel consists of 32 bits. 24-bits of these are for RGB values,
      leaving 8 bits unused.

      We'll use this 8 bits to store an "error", whereby an error of 0 means
      that the pixel is computed perfectly and does not need to be recalculated.

      However when we're zooming and scaling, we gradually accumulate more and
      more errors, and when we render pixels at low resolution (16x16), we'll
      make sure to tag those pixels which are wrong so that they'll be
      recomputed or redisplayed at the appropriate time.

      When we scale or scroll the viewport, we'll copy some of the original
      pixels over to the new view to give the viewer something to look at whilst
      we calculate it properly.
    */

    std::vector<RGB> new_contents(vp.width * vp.height, grey);
    bool zoom_eq = r == 1.0;
    bool zoom_out = r > 1.0;

    for (int j = 0; j < vp.height; ++j)
      for (int i = 0; i < vp.width; ++i) {
        int i2 = r * i + dx;
        int j2 = r * j + dy;
        if (i2 >= 0 && i2 < vp.width && j2 >= 0 && j2 < vp.height) {
          auto orig = vp(i2, j2);
          new_contents[i + j * vp.width] =
              zoom_eq ? orig
                      : with_extra(orig, zoom_out ? 20 : extra(orig) + 1);
        } else
          new_contents[i + j * vp.width] = grey;
      }

    std::copy(new_contents.begin(), new_contents.end(), vp.data);
  }

  void redraw(Viewport &vp) override {
    stop_current_calculation();
    Renderer::redraw(vp);
  }

  void set_aspect_ratio(int new_width, int new_height) override {
    stop_current_calculation();
  }

  bool set_coords(const view_coords &c, Viewport &vp) override {
    redraw(vp);
    coords = c; // TODO: Update aspect ratio
    return true;
  }

  void scroll(int dx, int dy, Viewport &vp) override {
    stop_current_calculation();

    // TODO: Only recalculate necessary regions
    coords = coords.scroll(vp.width, vp.height, dx, dy);

    remap_viewport(vp, dx, dy, 1.0);

    vp.region_updated(0, 0, vp.width, vp.height);
  }

  bool automaticallyAdjustDepth = true;

  void enable_auto_depth(bool value) override {
    automaticallyAdjustDepth = value;
  }

  void set_threading(int threads) override { this->threads = threads; }

  void get_depth_range(double &min, double &p, double &max) override {
    stop_current_calculation();
    min = view_min;
    max = view_max;
    p = view_percentile_max;
  }
};
} // namespace fractals