#include "Renderer.hpp"
#include "ColourMap.hpp"
#include "RenderingSequence.hpp"
#include "ViewCoords.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"

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
  int previous_stride = rs.stride;
  double min_depth = 0, max_depth = 0;

  do {
    if (rs.stride != previous_stride) {
      // if (rs.stride <= 1)
      vp.region_updated(0, 0, w, h);
      previous_stride = rs.stride;
    }
    auto &point = vp(rs.x, rs.y);

    if (extra(point)) {
      auto depth = calculate_point(w, h, rs.x, rs.y);
      if (depth < min_depth || min_depth == 0)
        min_depth = depth;
      if (depth > max_depth || max_depth == 0)
        max_depth = depth;
      point = cm(depth);
    }

#if 1
    if (rs.stride > 1 && rs.x > 0 && rs.y > 0) {
      // Interpolate the region
      interpolate_region(vp, rs.x - rs.stride, rs.y - rs.stride, rs.stride);
    }
#endif
    if (stop)
      return;
  } while (rs.next());

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

  vp.finished(width(), min_depth, max_depth, d.count());
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
        // TODO: Only redraw black points
        vp(i, j) = with_extra(vp(i, j), 127);
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

  void set_fractal(const fractals::PointwiseFractal &f) override {
    stop_current_calculation();
    underlying_fractal->set_fractal(f);
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

  void calculate_region_in_thread(fractals::Viewport &vp, const ColourMap &cm,
                                  std::atomic<bool> &stop, int x0, int y0,
                                  int w, int h) {

    RenderingSequence rs(w, h, 16);
    int previous_stride = rs.stride;
    double min_depth = 0, max_depth = 0;

    do {
      if (rs.stride != previous_stride) {
        // if (rs.stride <= 4)
        vp.region_updated(x0, y0, w, h);
        previous_stride = rs.stride;
      }
      auto &point = vp(x0 + rs.x, y0 + rs.y);

      if (extra(point)) {
        auto depth = calculate_point(vp.width, vp.height, x0 + rs.x, y0 + rs.y);
        if (depth < min_depth || min_depth == 0)
          min_depth = depth;
        if (depth > max_depth || max_depth == 0)
          max_depth = depth;
        point = cm(depth);
      }

#if 1
      if (rs.stride > 1 && rs.x > 0 && rs.y > 0) {
        // Interpolate the region
        interpolate_region(vp, rs.x - rs.stride, rs.y - rs.stride, rs.stride);
      }
#endif
      if (stop)
        return;
    } while (rs.next());

    if (view_min == 0 || min_depth > 0 && min_depth < view_min)
      view_min = min_depth;
    if (view_max == 0 || max_depth > view_max)
      view_max = max_depth;

    vp.region_updated(x0, y0, w, h);
    // vp.finished(width(), min_depth, max_depth, d.count());
  }

  std::atomic<double> view_min, view_max;
  int threads = 1;

  void calculate_async(fractals::Viewport &view, const ColourMap &cm) override {
    stop_current_calculation();

    stop = false;

    ++active_threads;
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

              view.finished(width(), view_min, view_max, d.count());
            }
            // Notify that we are finished
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

  ViewCoords get_coords() const override {
    return underlying_fractal->get_coords();
  }

  ViewCoords initial_coords() const override {
    return underlying_fractal->initial_coords();
  }

  int iterations() const override { return underlying_fractal->iterations(); }

  bool set_coords(const ViewCoords &c, Viewport &vp) override {
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

void Viewport::region_updated(int x, int y, int w, int h) {}

void fractals::Viewport::finished(double, int, int, double) {}

template <typename LowPrecisionComplex, typename HighPrecisionComplex>
struct test_algorithm {
  struct params_type {
    params_type(const ViewCoords &v);
  };

  struct view_type {
    view_type(const params_type &pt, std::atomic<bool> &stop);
  };

  static double calculate(const view_type &view, int x, int y);

  using next_type = void;
};

class CalculatedFractalRenderer : public fractals::Renderer {
public:
  ViewCoords coords;

  CalculatedFractalRenderer(const PointwiseFractal &f) : factory(&f) {
    coords = initial_coords();
  }

  ViewCoords get_coords() const override { return coords; }

  bool set_coords(const ViewCoords &w, Viewport &vp) override {
    coords = w; // TODO: Update aspect ratio
    return true;
  }

  void start_async_calculation(Viewport &vp, std::atomic<bool> &stop) override {
    calculation = factory->create(coords, vp.width, vp.height, stop);
  }

  double calculate_point(int w, int h, int x, int y) override {
    return calculation->calculate(x, y);
  }

  void increase_iterations(Viewport &vp) override {
    coords.max_iterations *= 2;
  }

  // Also marks the relevant pixels to redraw in the viewport
  void decrease_iterations(Viewport &vp) override {
    coords.max_iterations /= 2;
  }

  bool zoom(double r0, int cx, int cy, Viewport &vp) override {

    double pw = vp.width;
    double ph = vp.height;

    auto pixel_width =
        vp.width > vp.height ? coords.r * (2.0 / ph) : coords.r * (2.0 / pw);

    auto point_size =
        vp.width > vp.height ? coords.r * (2.0 / ph) : coords.r * (2.0 / pw);

    ViewCoords::value_type r{r0};

    auto CX = coords.x + pixel_width * (cx - pw / 2);
    auto CY = coords.y + pixel_width * (cy - ph / 2);

    ViewCoords new_coords;
    new_coords.max_iterations = coords.max_iterations;
    new_coords.x = CX - (CX - coords.x) * r;
    new_coords.y = CY - (CY - coords.y) * r;
    new_coords.r = coords.r * r;

    if (!factory->valid_for(new_coords)) {
      return false;
    }

    coords = new_coords;
    return true;
  }

  void scroll(int dx, int dy, Viewport &vp) override {
    if (vp.width > vp.height) {
      coords.y += coords.r * (2.0 * dy / vp.height);
      coords.x += coords.r * (2.0 * dx / vp.height);
    } else {
      coords.y += coords.r * (2.0 * dy / vp.width);
      coords.x += coords.r * (2.0 * dx / vp.width);
    }
  }

  double width() const override { return convert<double>(coords.r); }

  void set_aspect_ratio(Viewport &vp) override {}

  int iterations() const override { return coords.max_iterations; }

  ViewCoords initial_coords() const override {
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

extern const fractals::PointwiseFractal &mb;

std::unique_ptr<fractals::Renderer> fractals::make_renderer() {
  return std::make_unique<AsyncRenderer>(
      std::make_unique<CalculatedFractalRenderer>(mb));
}
