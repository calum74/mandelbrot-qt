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
#include <deque>
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

  // If all 4 corners have the same colour, claim that the center is accurate
  // 1 means more speed
  // 0 means more accuracy
#if 1
  if (c00 == c10 && c00 == c11 && c00 == c01 && h <= 4) {
    for (int j = 0; j < h; ++j)
      for (int i = 0; i < h; ++i)
        vp(x0 + i, y0 + j) = c00;
  }
#endif

#if 1
  // Solid colour
  for (int j = 0; j <= h; ++j)
    for (int i = 0; i <= h; ++i) {
      if ((i < h || j < h) && (i > 0 || j > 0)) {
        auto &p = vp(x0 + i, y0 + j);
        if (extra(p) > h)
          p = with_extra(c11, h);
      }
    }
  return;
#endif

#if 0
  // Linear interpolation (also weird) - delete this
  for (int j = 0; j < h; ++j)
    for (int i = 0; i < h; ++i) {
#if 0
      int r = red(c00) * (h - j) * (h - i) + red(c10) * i * (h - j) +
              red(c01) * (h - i) * j + red(c11) * i * j;
      int g = green(c00) * (h - j) * (h - i) + green(c10) * i * (h - j) +
              green(c01) * (h - i) * j + green(c11) * i * j;
      int b = blue(c00) * (h - j) * (h - i) + blue(c10) * i * (h - j) +
              blue(c01) * (h - i) * j + blue(c11) * i * j;
      auto c = make_rgb(r / (h * h), g / (h * h), b / (h * h));
#endif

      auto c0 = blend(c00, c10, h - i, i);
      auto c1 = blend(c01, c11, h - i, i);
      auto c = blend(c0, c1, h - j, j);

      // if (i > 0 && j > 0)
      //  c = c00;
      vp(x0 + i, y0 + j) = with_extra(c, std::abs(h - i) + std::abs(h - j));
    }
#endif
}

void fractals::Renderer::calculate(Viewport &vp, const ColourMap &cm,
                                   std::atomic<bool> &stop) {
  int w = vp.width;
  int h = vp.height;
  auto t0 = std::chrono::high_resolution_clock::now();

  int depth_range = 500;

  RenderingSequence rs(w, h, 16);
  double min_depth = 0, max_depth = 0;
  int x, y, stride;
  bool stride_changed = false;

  while (rs.next(x, y, stride, stride_changed)) {
    if (stride_changed) {
      vp.region_updated(0, 0, w, h);
    }
    auto &point = vp(x, y);

    if (extra(point)) {
      auto depth = calculate_point(w, h, x, y);
      if (depth < min_depth || min_depth == 0)
        min_depth = depth;
      if (depth > max_depth || max_depth == 0)
        max_depth = depth;
      point = cm(depth);
    }

#if 1
    if (stride > 1 && x > 0 && y > 0) {
      // Interpolate the region
      interpolate_region(vp, x - stride, y - stride, stride);
    }
#endif
    if (stop)
      return;
  }

#if 0
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      vp(x, y) = calculate_point(w, h, x, y);
      if (stop)
        return;
    }
#endif

  vp.region_updated(0, 0, vp.width, vp.height);

  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> d = t1 - t0;

  vp.finished(width(), min_depth, max_depth, get_average_iterations(),
              get_average_skipped_iterations(), d.count());
}

void Renderer::increase_iterations(Viewport &) {}

void Renderer::decrease_iterations(Viewport &) {}

void Renderer::redraw(Viewport &vp) {
  for (int j = 0; j < vp.height; ++j)
    for (int i = 0; i < vp.width; ++i)
      vp(i, j) = with_extra(vp(i, j), 127);
}

class AsyncRenderer : public Renderer {
  std::unique_ptr<Renderer> underlying_fractal;
  std::vector<std::future<void>> calculate_threads;
  std::atomic<bool> stop;

  std::atomic<int> active_threads;

  std::chrono::time_point<std::chrono::high_resolution_clock> t0;

public:
  AsyncRenderer(std::unique_ptr<Renderer> underlying_fractal)
      : underlying_fractal{std::move(underlying_fractal)} {}

  ~AsyncRenderer() { stop_current_calculation(); }

  void calculate(fractals::Viewport &view, const ColourMap &cm,
                 std::atomic<bool> &) override {
    underlying_fractal->calculate(view, cm, stop);
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
    while (!calculate_threads.empty()) {
      calculate_threads.back().wait();
      calculate_threads.pop_back();
    }
  }

  void start_async_calculation(Viewport &vp, std::atomic<bool> &stop) override {
    underlying_fractal->start_async_calculation(vp, stop);
  }

  // We're going hold an array of all non-zero depths
  std::mutex depth_mutex;
  std::vector<double> depths;           // Guarded by depth_mutex
  RenderingSequence rendering_sequence; // Guarded by depth_mutex

  bool next_rendering_sequence(int &x, int &y, int &stride,
                               bool &stride_changed) {
    std::lock_guard<std::mutex> lck(depth_mutex);
    return rendering_sequence.next(x, y, stride, stride_changed);
  }

  void calculate_region_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop, int x0, int y0,
                                  int w, int h) {

    int x, y, stride;
    bool stride_changed = false;
    double min_depth = 0, max_depth = 0;

    while (next_rendering_sequence(x, y, stride, stride_changed)) {
      if (stride_changed) {
        // if (rs.stride <= 4)
        vp.region_updated(x0, y0, w, h);
      }
      auto &point = vp(x0 + x, y0 + y);

      /* if (extra(point)) */ {
        auto depth = calculate_point(vp.width, vp.height, x0 + x, y0 + y);
        if (depth < min_depth || min_depth == 0)
          min_depth = depth;
        if (depth > max_depth || max_depth == 0)
          max_depth = depth;
        std::lock_guard<std::mutex> lck(depth_mutex);
        ((std::atomic<int> &)point) = cm(depth);
        // point = cm(depth);
        if (depth > 0) {
          depths.push_back(depth);
        }
      }

#if 1
      if (stride > 1 && x > 0 && y > 0) {
        // Interpolate the region
        interpolate_region(vp, x - stride, y - stride, stride);
      }
#endif
      if (stop)
        return;
    }

    // Not threadsafe (FIXME)
    if (view_min == 0 || min_depth > 0 && min_depth < view_min)
      view_min = min_depth;
    if (view_max == 0 || max_depth > view_max)
      view_max = max_depth;

    // vp.region_updated(x0, y0, w, h);
    // vp.finished(width(), min_depth, max_depth, d.count());
  }

  std::atomic<double> view_min, view_max;
  int threads = 1;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override {
    stop_current_calculation();
    depths.clear();

    stop = false;

    ++active_threads;
    rendering_sequence = RenderingSequence(view.width, view.height, 16);

    calculate_threads.push_back(std::async([&]() {
      underlying_fractal->start_async_calculation(view, stop);

      t0 = std::chrono::high_resolution_clock::now();
      view_min = 0;
      view_max = 0;
      for (int t = 0; !stop && t < threads; ++t) {
        ++active_threads;
        calculate_threads.push_back(std::async([&, t]() {
          auto y0 = t * view.height / threads;
          auto y1 = (t + 1) * view.height / threads;
          calculate_region_in_thread(view, cm, stop, 0, y0, view.width,
                                     y1 - y0);
          if (0 == --active_threads) {
            if (!stop) {
              view.region_updated(0, 0, view.width, view.height);
              auto t1 = std::chrono::high_resolution_clock::now();
              std::chrono::duration<double> d = t1 - t0;

              view.finished(
                  width(), view_min, view_max,
                  underlying_fractal->get_average_iterations(),
                  underlying_fractal->get_average_skipped_iterations(),
                  d.count());
            }

            if (depths.begin() < depths.end()) {
              auto discovered_depth =
                  util::top_percentile(depths.begin(), depths.end(), 0.999);
              view.discovered_depth(std::distance(depths.begin(), depths.end()),
                                    *discovered_depth);
            }
          }
        }));
      }
      --active_threads;
    }));
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
};

void Renderer::calculate_async(fractals::Viewport &view, const ColourMap &cm) {
  std::atomic<bool> stop = false;
  calculate(view, cm, stop);
  if (!stop)
    view.region_updated(0, 0, view.width, view.height);
}

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

class HighPrecisionRenderer : public Renderer {
public:
  HighPrecisionRenderer(const fractals::PointwiseFractal &f)
      : current_fractal(&f) {
    coords = f.initial_coords();
  }

  ~HighPrecisionRenderer() { stop_calculation(); }

  std::future<layer> layer_calculation;

  void display_layer(Viewport &vp, const ColourMap &cm) {

    // Compute a mapping from the layer coords to the current coords

    // Assume that the current coords are embedded in the layer's coords
    auto &lc = layers.back().coords;

    auto ps = lc.point_size(vp.width, vp.height);
    auto x0 = (coords.left(vp.width, vp.height) - lc.left(vp.width, vp.height))
                  .to_double() /
              ps;
    auto y0 = (coords.top(vp.width, vp.height) - lc.top(vp.width, vp.height))
                  .to_double() /
              ps;
    auto r = coords.r.to_double() / lc.r.to_double();

    // std::cout << "Rendering using layer " << layers.size() << std::endl;
    // std::cout << "Need to project the coords " << x0 << "," << y0 << "-"
    //           << (vp.width * r) << "," << (vp.height * r) << std::endl;

    // Map the relevant layer to the viewport
    for (int j = 0; j < vp.height; ++j)
      for (int i = 0; i < vp.width; ++i) {
        int x = x0 + i * r;
        int y = y0 + j * r;

        // Should not happen but sensible to check
        if (x < 0 || x >= vp.width || y < 0 || y >= vp.height)
          vp(i, j) = 0;
        else {
          // TODO: Blend the pixels smoothly
          // Make sure we treat 0 as special
          vp(i, j) = cm(layers.back()(x0 + i * r, y0 + j * r));
        }
      }
    vp.region_updated(0, 0, vp.width, vp.height);
    vp.finished(0, 0, 0, 0, 0, 0);
  }

  std::atomic<bool> stop;

  void stop_calculation() {
    if (layer_calculation.valid()) {
      stop = true;
      layer_calculation.get();
      stop = false;
    }
  }

  void start_new_layer(const view_coords &new_coords, Viewport &vp, int cx,
                       int cy) {
    std::cout << "Starting layer " << layers.size() << " at " << new_coords
              << "\n";
    stop_calculation();
    layer_cx = cx;
    layer_cy = cy;

    layer_calculation = std::async(std::launch::async, [&vp, this, new_coords,
                                                        cx, cy]() {
      std::cout << "  Calculating layer\n";
      auto x = current_fractal->create(new_coords, vp.width, vp.height, stop);
      auto t0 = std::chrono::high_resolution_clock::now();
      auto l = layer(new_coords, cx, cy, vp.width, vp.height, *x, stop, 4);
      auto t1 = std::chrono::high_resolution_clock::now(); // !! Compute this in
                                                           // the layer itself
      std::chrono::duration<double> d = t1 - t0;

      std::cout << "  Layer calculation completed in " << d.count()
                << " seconds\n";
      return l;
    });
  }

  void calculate_async(Viewport &vp, const ColourMap &cm) override {

    // std::cout << "Call to calculate_async\n";
    if (!layer_calculation.valid() && layers.empty()) {
      start_new_layer(coords, vp, 0, 0);
      std::cout << "Pushed initial layer\n";
      layers.push_back(layer_calculation.get());
    }
    display_layer(vp, cm);
  }

  void start_async_calculation(Viewport &vp, std::atomic<bool> &stop) override {
  }

  double calculate_point(int w, int h, int x, int y) override { return 0; }

  int layer_cx, layer_cy;

  void start_new_layer(Viewport &vp, int cx, int cy) {
    /*
      Note the cx,cy are relative to the current coords, not the outer coords.

      1. Locate the fixed point (CX,CY) that we'll keep fixed.
      2. Zoom in by a factor of 2 from the outer layer, keeping the fixed point
        (CX,CY) in the same position.

    */

    auto C = coords.map_point(vp.width, vp.height, cx, cy);

    start_new_layer(layers.back().coords.zoom(0.50, vp.width, vp.height, cx, cy,
                                              C.first, C.second),
                    vp, cx, cy);
  }

  bool zoom(double r, int cx, int cy, Viewport &vp) override {

    // In quality mode, we'll slow down the zooming speed
    r = std::pow(r, 0.25);

    if (r < 0.5)
      r = 0.5;
    if (r > 2)
      r = 2;

    auto layer_ratio =
        r * coords.r.to_double() / layers.back().coords.r.to_double();
    std::cout << "Layer ratio = " << layer_ratio << std::endl;

    // r is the new size relative to the old size
    // When we zoom, we lock in a point, and zoom to that.

    while (layer_ratio < 0.5) {
      // We need to push a new layer

      if (!layer_calculation.valid()) {
        // We need to start calculating the next layer, and lock in the
        //  coordinates
        start_new_layer(vp, cx, cy);
      }

      // Push the new layer

      auto t1 = std::chrono::high_resolution_clock::now();
      std::cout << "   waiting for calculation...\n";

      layers.push_back(layer_calculation.get());
      layer_cx = layers.back().layer_cx;
      layer_cy = layers.back().layer_cy;

      auto t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> d = t2 - t1;
      std::cout << "   Blocked for " << d.count() << " seconds\n";

      layer_ratio *= 2;

      // Start the next calculation
      start_new_layer(vp, cx, cy);
    }

    while (layer_ratio > 1.0 && layers.size() > 1) {
      // Need to instead pop layers
      std::cout << "Popping layer " << layers.size() << std::endl;
      stop_calculation();
      layer_cx = layers.back().layer_cx;
      layer_cy = layers.back().layer_cy;
      layers.pop_back();
      layer_ratio *= 0.5;
    }

    if (r < 1 && !layer_calculation.valid()) {
      // We still need to start a new calculation to avoid blocking the zoom
      // What point do we want to fix?
      // We want to fix the point at (cx,cy), and zoom in by a factor of 2 from
      // the outer layer.
      start_new_layer(vp, cx, cy);
    }

    coords = coords.zoom(r, vp.width, vp.height, cx, cy);
    return true;
  }

  void scroll(int dx, int dy, Viewport &vp) override {}

  void set_aspect_ratio(Viewport &vp) override {}

  double width() const override { return coords.r.to_double(); }

  view_coords get_coords() const override { return coords; }

  bool set_coords(const view_coords &c, Viewport &vp) override {
    coords = c;
    return true;
  }

  int iterations() const override { return coords.max_iterations; }

  view_coords initial_coords() const override {
    return current_fractal->initial_coords();
  }

private:
  const fractals::PointwiseFractal *current_fractal;
  view_coords coords;
  std::deque<fractals::layer> layers;
};

std::unique_ptr<fractals::Renderer> fractals::make_renderer() {
  return std::make_unique<AsyncRenderer>(
      std::make_unique<CalculatedFractalRenderer>(mandelbrot_fractal));
}
