#include "Renderer.hpp"
#include "ColourMap.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "high_exponent_real.hpp"
#include "mandelbrot.hpp"
#include "percentile.hpp"
#include "registry.hpp"
#include "rendering_sequence.hpp"
#include "view_coords.hpp"
#include "view_parameters.hpp"

#include <cassert>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using namespace fractals;

// !! Not used
RGB blend(RGB c1, RGB c2, int w1, int w2) {
  return make_rgb((red(c1) * w1 + red(c2) * w2) / (w1 + w2),
                  (green(c1) * w1 + green(c2) * w2) / (w1 + w2),
                  (blue(c1) * w1 + blue(c2) * w2) / (w1 + w2));
}

void interpolate_region(Viewport &vp, int x0, int y0, int h) {
  auto c00 = vp(x0, y0);
  auto c10 = vp(x0 + h, y0);
  auto c01 = vp(x0, y0 + h);
  auto c11 = vp(x0 + h, y0 + h);
  auto average = blend(blend(c00, c10, 1, 1), blend(c01, c11, 1, 1), 1, 1);

  // If all 4 corners have the same colour, claim that the filled in colour is
  // accurate and does not need to be recalculated 1 means more speed 0 means
  // more accuracy
#if 1
  if (c00 == c10 && c00 == c11 && c00 == c01 && h <= 4) {
    for (int j = 0; j < h; ++j)
      for (int i = 0; i < h; ++i)
        vp(x0 + i, y0 + j) = c00;
  }
#endif

  // Solid colour
  for (int j = 0; j < h; ++j)
    for (int i = 0; i < h; ++i) {
      int error = (i == 0 || i == h) && (j == 0 || j == h) ? 0 : i + j;
      /* if ((i < h || j < h) && (i > 0 || j > 0)) */
      auto &p = vp(x0 + i, y0 + j);
      if (error < extra(p))
        p = with_extra(c11, error);
    }
}

void Renderer::increase_iterations(Viewport &) {}

void Renderer::decrease_iterations(Viewport &) {}

void Renderer::center(Viewport &vp) {}

void Renderer::zoom_in(Viewport &vp) {}

void Renderer::redraw(Viewport &vp) {
  for (int j = 0; j < vp.height; ++j)
    for (int i = 0; i < vp.width; ++i)
      vp(i, j) = with_extra(vp(i, j), 127);
}

void Renderer::enable_auto_depth(bool value) {}

void Renderer::set_threading(int threads) {}

class AsyncRenderer : public Renderer {

  const PointwiseFractal *current_fractal;
  view_coords coords;
  Registry &registry;
  std::future<void> current_calculation;
  std::atomic<bool> stop;
  std::shared_ptr<PointwiseCalculation> calculation;

  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

public:
  AsyncRenderer(const PointwiseFractal &fractal, Registry &registry)
      : current_fractal(&fractal), registry(registry) {
    coords = initial_coords();
  }

  ~AsyncRenderer() { stop_current_calculation(); }

  void load(const view_parameters &params, Viewport &vp) override {
    stop_current_calculation();

    coords = params.coords;
    auto new_fractal = registry.lookup(params.fractal_name);

    if (new_fractal)
      current_fractal = new_fractal;

    redraw(vp);
  }

  void save(view_parameters &params) const override {
    params.coords = coords;
    params.fractal_name = current_fractal->name();
  }

  void increase_iterations(Viewport &vp) override {
    stop_current_calculation();
    for (int j = 0; j < vp.height; ++j)
      for (int i = 0; i < vp.width; ++i) {
        auto &c = vp(i, j);
        if (!c) {
          // Only redraw final coloured points
          c = with_extra(vp(i, j), 127);
        }
      }
    coords.max_iterations *= 2;
  }

  void decrease_iterations(Viewport &vp) override {
    stop_current_calculation();
    for (int j = 0; j < vp.height; ++j)
      for (int i = 0; i < vp.width; ++i) {
        vp(i, j) = with_extra(vp(i, j), 127);
      }
    coords.max_iterations /= 2;
  }

  double get_average_iterations() const override {
    return calculation->average_iterations();
  }

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
        return 0.0 / 0.0;
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

void Renderer::get_depth_range(double &min, double &p, double &max) {}

void Renderer::calculate_async(fractals::Viewport &view, const ColourMap &cm) {}

double Renderer::get_average_iterations() const { return 0; }

double Renderer::get_average_skipped_iterations() const { return 0; }

void Viewport::region_updated(int x, int y, int w, int h) {}

void Viewport::discovered_depth(int, double) {}

void fractals::Viewport::finished(double, int, int, double, double, double) {}

void Renderer::discovered_depth(int, double) {}

class CalculatedFractalRenderer : public fractals::Renderer {
public:
private:
};

void fractals::Renderer::set_fractal(const fractals::PointwiseFractal &) {}

void fractals::Viewport::invalidateAllPixels() {
  for (auto j = 0; j < height; ++j)
    for (auto i = 0; i < width; ++i) {
      (*this)(i, j) = with_extra((*this)(i, j), 127);
    }
}

void Renderer::load(const view_parameters &params, Viewport &vp) {}

void Renderer::save(view_parameters &params) const {}

std::unique_ptr<fractals::Renderer> fractals::make_renderer(Registry &reg) {
  return std::make_unique<AsyncRenderer>(mandelbrot_fractal, reg);
}
