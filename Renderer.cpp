#include "Renderer.hpp"
#include "ColourMap.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "layer.hpp"
#include "mandelbrot.hpp"
#include "percentile.hpp"
#include "rendering_sequence.hpp"
#include "view_coords.hpp"

#include <cassert>
#include <future>
#include <memory>
#include <vector>

using namespace fractals;

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
  for (int j = 0; j <= h; ++j)
    for (int i = 0; i <= h; ++i) {
      if ((i < h || j < h) && (i > 0 || j > 0)) {
        auto &p = vp(x0 + i, y0 + j);
        if (extra(p) > h)
          p = with_extra(c11, h);
      }
    }
}

void Renderer::increase_iterations(Viewport &) {}

void Renderer::decrease_iterations(Viewport &) {}

void Renderer::redraw(Viewport &vp) {
  for (int j = 0; j < vp.height; ++j)
    for (int i = 0; i < vp.width; ++i)
      vp(i, j) = with_extra(vp(i, j), 127);
}

void Renderer::enableAutoDepth(bool value) {}

void Renderer::setThreading(int threads) {}

class AsyncRenderer : public Renderer {
  std::unique_ptr<Renderer> underlying_fractal;
  std::future<void> current_calculation;
  std::atomic<bool> stop;

  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

public:
  AsyncRenderer(std::unique_ptr<Renderer> underlying_fractal)
      : underlying_fractal{std::move(underlying_fractal)} {}

  ~AsyncRenderer() { stop_current_calculation(); }

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
    underlying_fractal->increase_iterations(vp);
  }

  void decrease_iterations(Viewport &vp) override {
    stop_current_calculation();
    for (int j = 0; j < vp.height; ++j)
      for (int i = 0; i < vp.width; ++i) {
        vp(i, j) = with_extra(vp(i, j), 127);
      }
    underlying_fractal->decrease_iterations(vp);
  }

  double get_average_iterations() const override {
    return underlying_fractal->get_average_iterations();
  }

  double get_average_skipped_iterations() const override {
    return underlying_fractal->get_average_skipped_iterations();
  }

  void set_fractal(const fractals::PointwiseFractal &f) override {
    stop_current_calculation();
    underlying_fractal->set_fractal(f);
  }

  void discovered_depth(int points, double discovered_depth) override {
    underlying_fractal->discovered_depth(points, discovered_depth);
  }

  double calculate_point(int w, int h, int x, int y) override {
    return underlying_fractal->calculate_point(w, h, x, y);
  }

  void stop_current_calculation() {
    stop = true;
    if (current_calculation.valid())
      current_calculation.get();
    stop = false;
  }

  void start_async_calculation(Viewport &vp, std::atomic<bool> &stop) override {
    underlying_fractal->start_async_calculation(vp, stop);
  }

  // We're going hold an array of all non-zero depths
  std::vector<double> depths;

  class my_rendering_sequence
      : public fractals::buffered_rendering_sequence<double> {

  public:
    my_rendering_sequence(Renderer &underlying_fractal, const ColourMap &cm,
                          Viewport &vp)
        : fractals::buffered_rendering_sequence<double>(vp.width, vp.height,
                                                        16),
          underlying_fractal(underlying_fractal), cm(cm), vp(vp) {}

    double min_depth = 0, max_depth = 0;
    std::vector<double> depths;

    void layer_complete(int stride) override {
      // Transfer and interpolate to the current viewport

      fractals::rendering_sequence seq(vp.width, vp.height, 16);
      seq.start_at_stride(stride);
      int x, y, s;
      bool c;
      while (seq.next(x, y, s, c) && stride == s) {
        double depth = output[x + y * vp.width];
        vp(x, y) = cm(depth);
        if (depth > 0) {
          depths.push_back(depth);
          if (depth > max_depth)
            max_depth = depth;
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

      vp.region_updated(0, 0, vp.width, vp.height);
    }

    double get_point(int x, int y) override {
      return underlying_fractal.calculate_point(vp.width, vp.height, x, y);
    }

  private:
    Renderer &underlying_fractal;
    const ColourMap &cm;
    Viewport &vp;
  };

  int threads = 4;

  void calculate_region_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop) {

    my_rendering_sequence seq(*underlying_fractal, cm, vp);
    seq.calculate(threads, stop);
    view_min = seq.min_depth;
    view_max = seq.max_depth;
    depths = std::move(seq.depths);
  }

  double view_min, view_max;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override {
    stop_current_calculation();
    depths.clear();

    stop = false;

    current_calculation = std::async([&]() {
      underlying_fractal->start_async_calculation(view, stop);

      t0 = std::chrono::high_resolution_clock::now();
      view_min = 0;
      view_max = 0;
      calculate_region_in_thread(view, cm, stop);
      if (!stop) {
        view.region_updated(0, 0, view.width, view.height);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> d = t1 - t0;

        view.finished(width(), view_min, view_max,
                      underlying_fractal->get_average_iterations(),
                      underlying_fractal->get_average_skipped_iterations(),
                      d.count());
      }

      if (automaticallyAdjustDepth && depths.begin() < depths.end()) {
        auto discovered_depth =
            util::top_percentile(depths.begin(), depths.end(), 0.999);
        view.discovered_depth(std::distance(depths.begin(), depths.end()),
                              *discovered_depth);
      }
    });
  }

  bool zoom(double r, int cx, int cy, Viewport &vp) override {
    stop_current_calculation();

    if (underlying_fractal->zoom(r, cx, cy, vp)) {

      remap_viewport(vp, cx * (1 - r), cy * (1 - r), r);

      vp.region_updated(0, 0, vp.width, vp.height);
      return true;
    }
    return false;
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

  void set_aspect_ratio(Viewport &vp) override {
    redraw(vp);
    // TODO: Transfer pixels from old to the new pixmap
    underlying_fractal->set_aspect_ratio(vp);
  }

  view_coords get_coords() const override {
    return underlying_fractal->get_coords();
  }

  view_coords initial_coords() const override {
    return underlying_fractal->initial_coords();
  }

  int iterations() const override { return underlying_fractal->iterations(); }

  bool set_coords(const view_coords &c, Viewport &vp) override {
    redraw(vp);
    return underlying_fractal->set_coords(c, vp);
  }

  void scroll(int dx, int dy, Viewport &vp) override {
    stop_current_calculation();

    // TODO: Only recalculate necessary regions
    underlying_fractal->scroll(dx, dy, vp);

    remap_viewport(vp, dx, dy, 1.0);

    vp.region_updated(0, 0, vp.width, vp.height);
  }

  double width() const override { return underlying_fractal->width(); }

  bool automaticallyAdjustDepth = true;

  void enableAutoDepth(bool value) override {
    automaticallyAdjustDepth = value;
  }

  void setThreading(int threads) override { this->threads = threads; }
};

void Renderer::calculate_async(fractals::Viewport &view, const ColourMap &cm) {}

double Renderer::get_average_iterations() const { return 0; }

double Renderer::get_average_skipped_iterations() const { return 0; }

void Viewport::region_updated(int x, int y, int w, int h) {}

void Viewport::discovered_depth(int, double) {}

void fractals::Viewport::finished(double, int, int, double, double, double) {}

void Renderer::discovered_depth(int, double) {}

class CalculatedFractalRenderer : public fractals::Renderer {
public:
  view_coords coords;

  CalculatedFractalRenderer(const PointwiseFractal &f) : factory(&f) {
    coords = initial_coords();
  }

  view_coords get_coords() const override { return coords; }

  bool set_coords(const view_coords &w, Viewport &vp) override {
    coords = w; // TODO: Update aspect ratio
    return true;
  }

  void start_async_calculation(Viewport &vp, std::atomic<bool> &stop) override {
    calculation = factory->create(coords, vp.width, vp.height, stop);
  }

  double get_average_iterations() const override {
    return calculation->average_iterations();
  }

  double get_average_skipped_iterations() const override {
    return calculation->average_skipped();
  }

  double calculate_point(int w, int h, int x, int y) override {
    return calculation->calculate(x, y);
  }

  void discovered_depth(int points, double discovered_depth) override {
    if (points > 1000)                              // Fudge factor
      coords.max_iterations = discovered_depth * 2; // Fudge factor
  }

  void increase_iterations(Viewport &vp) override {
    coords.max_iterations *= 2;
  }

  // Also marks the relevant pixels to redraw in the viewport
  void decrease_iterations(Viewport &vp) override {
    coords.max_iterations /= 2;
  }

  bool zoom(double r0, int cx, int cy, Viewport &vp) override {

    auto new_coords = coords.zoom(r0, vp.width, vp.height, cx, cy);

    if (!factory->valid_for(new_coords)) {
      return false;
    }

    coords = new_coords;
    return true;
  }

  void scroll(int dx, int dy, Viewport &vp) override {
    coords = coords.scroll(vp.width, vp.height, dx, dy);
  }

  double width() const override { return convert<double>(coords.r); }

  void set_aspect_ratio(Viewport &vp) override {}

  int iterations() const override { return coords.max_iterations; }

  view_coords initial_coords() const override {
    return factory->initial_coords();
  }

  void set_fractal(const fractals::PointwiseFractal &f) override {
    factory = &f;
  }

private:
  const PointwiseFractal *factory;
  std::unique_ptr<PointwiseCalculation> calculation;
};

void fractals::Renderer::set_fractal(const fractals::PointwiseFractal &) {}

std::unique_ptr<fractals::Renderer> fractals::make_renderer() {
  return std::make_unique<AsyncRenderer>(
      std::make_unique<CalculatedFractalRenderer>(mandelbrot_fractal));
}
