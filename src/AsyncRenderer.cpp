#include "AsyncRenderer.hpp"

#include "ColourMap.hpp"
#include "Viewport.hpp"
#include "high_exponent_real.hpp"
#include "percentile.hpp"
#include "registry.hpp"
#include "view_parameters.hpp"

fractals::AsyncRenderer::AsyncRenderer(const PointwiseFractal &fractal,
                                       Registry &registry)
    : current_fractal(fractal.create()), registry(registry) {
  coords = initial_coords();
}

fractals::AsyncRenderer::~AsyncRenderer() { stop_current_calculation(); }

void fractals::AsyncRenderer::load(const view_parameters &params,
                                   Viewport &vp) {
  stop_current_calculation();

  coords = params.coords;
  auto new_fractal = registry.lookup(params.algorithm);

  if (new_fractal)
    current_fractal = new_fractal->create();

  redraw(vp);
}

void fractals::AsyncRenderer::save(view_parameters &params) const {
  params.coords = coords;
  params.algorithm = current_fractal->name();
}

void fractals::AsyncRenderer::increase_iterations(Viewport &vp) {
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

void fractals::AsyncRenderer::decrease_iterations(Viewport &vp) {
  stop_current_calculation();
  for (int j = 0; j < vp.height; ++j)
    for (int i = 0; i < vp.width; ++i) {
      vp(i, j) = with_extra(vp(i, j), 127);
    }
  coords.max_iterations /= 2;
}

double fractals::AsyncRenderer::get_average_iterations() const {
  return calculation->average_iterations();
}

double fractals::AsyncRenderer::get_average_skipped_iterations() const {
  return calculation->average_skipped();
}

void fractals::AsyncRenderer::discovered_depth(
    const RenderingMetrics &metrics) {
  if (metrics.non_black_points > 1000 &&
      metrics.discovered_depth > 0)                       // Fudge factor
    coords.max_iterations = metrics.discovered_depth * 2; // Fudge factor
}

void fractals::AsyncRenderer::set_fractal(const fractals::PointwiseFractal &f) {
  stop_current_calculation();
  current_fractal = f.create();
}

const char *fractals::AsyncRenderer::get_fractal_family() const {
  return current_fractal->family();
}

const char *fractals::AsyncRenderer::get_fractal_name() const {
  return current_fractal->name();
}

fractals::view_coords fractals::AsyncRenderer::initial_coords() const {
  return current_fractal->initial_coords();
}

void fractals::AsyncRenderer::stop_current_calculation() {
  stop = true;
  if (current_calculation.valid())
    current_calculation.get();
  stop = false;
}

void fractals::AsyncRenderer::calculate_region_in_thread(
    fractals::Viewport &vp, const ColourMap &cm, std::atomic<bool> &stop) {

  my_rendering_sequence seq(*calculation, cm, vp, depths);
  seq.calculate(threads, stop);

  if (seq.calculated_pixels > 100) {
    // !! I don't know when we sometimes return 1 or 0 pixels calculated
    metrics.min_depth = seq.min_depth;
    metrics.max_depth = seq.max_depth;
    metrics.points_calculated = seq.calculated_pixels;
  }
}

void fractals::AsyncRenderer::calculate_async(fractals::Viewport &view,
                                              const ColourMap &cm) {
  stop_current_calculation();
  depths.clear();

  stop = false;

  current_calculation = std::async([&]() {
    t0 = std::chrono::high_resolution_clock::now();

    calculation =
        current_fractal->create(coords, view.width, view.height, stop);

    metrics.min_depth = 0;
    metrics.max_depth = 0;
    metrics.discovered_depth = 0;
    calculate_region_in_thread(view, cm, stop);
    auto t1 = std::chrono::high_resolution_clock::now();

    metrics.non_black_points = std::distance(depths.begin(), depths.end());
    if (depths.begin() < depths.end()) {
      auto discovered_depth =
          util::top_percentile(depths.begin(), depths.end(), 0.999)->depth;
      metrics.discovered_depth = discovered_depth;
    } else {
      metrics.discovered_depth = metrics.max_depth;
    }

    std::chrono::duration<double> d = t1 - t0;
    metrics.render_time_seconds = d.count();
    metrics.log_radius = log_width();
    metrics.seconds_per_point =
        d.count() / metrics.points_calculated; // !! Redundant field

    if (stop) {
      metrics.fully_evaluated = false;
    } else {
      view.updated();
      metrics.fully_evaluated = true;
      metrics.average_iterations = calculation->average_iterations();
      metrics.average_skipped_iterations = calculation->average_skipped();
    }
    view.finished(metrics);
  });
}

bool fractals::AsyncRenderer::zoom(double r, int cx, int cy, bool lockCenter,
                                   Viewport &vp) {
  stop_current_calculation();

  auto new_coords =
      lockCenter ? coords.zoom(r) : coords.zoom(r, vp.width, vp.height, cx, cy);

  if (!current_fractal->valid_for(new_coords)) {
    return false;
  }

  coords = new_coords;

  remap_viewport(vp, cx * (1 - r), cy * (1 - r), r);

  vp.updated();
  return true;
}

fractals::view_coords fractals::AsyncRenderer::get_coords() const {
  return coords;
}

double fractals::AsyncRenderer::log_width() const {
  return fractals::log(convert<high_exponent_real<>>(coords.r));
}

int fractals::AsyncRenderer::iterations() const {
  return coords.max_iterations;
}

void fractals::AsyncRenderer::center(Viewport &vp) {
  if (max_x > 0 && max_y > 0)
    scroll(max_x - vp.width / 2, max_y - vp.height / 2, vp);
}

void fractals::AsyncRenderer::zoom_in(Viewport &vp) {
  zoom(0.5, vp.width / 2, vp.height / 2, true, vp);
}

bool fractals::AsyncRenderer::get_auto_zoom(int &x, int &y) {
  if (!depths.empty()) {
    auto p = util::top_percentile(depths.begin(), depths.end(), 0.9999);
    x = p->x;
    y = p->y;
    return true;
  }
  return false;
}

constexpr fractals::RGB grey = fractals::make_rgbx(100, 100, 100, 127);

namespace fractals {
RGB blend(RGB c1, RGB c2, double w1, double w2) {
  return make_rgb((red(c1) * w1 + red(c2) * w2) / (w1 + w2),
                  (green(c1) * w1 + green(c2) * w2) / (w1 + w2),
                  (blue(c1) * w1 + blue(c2) * w2) / (w1 + w2));
}
} // namespace fractals

void fractals::map_viewport(const Viewport &src, Viewport &dest, double dx,
                            double dy, double r) {

  // One day, we might be able to remap to a different size
  assert(src.width == dest.width);
  assert(src.height == dest.height);

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  for (int j = 0; j < dest.height; ++j)
    for (int i = 0; i < dest.width; ++i) {
      int i2 = r * i + dx;
      int j2 = r * j + dy;
      // int i2 = i2d;
      // int j2 = j2d;
      if (i2 >= 0 && i2 < dest.width && j2 >= 0 && j2 < dest.height) {
#if 0
        // This is so slow!!!
        auto i22 = i2 < dest.width - 1 ? i2 + 1 : i2;
        auto j22 = j2 < dest.height - 1 ? j2 + 1 : j2;
        auto p1 = src(i2, j2);
        auto p2 = src(i22, j2);
        auto p3 = src(i2, j22);
        auto p4 = src(i22, j22);
        auto di = i2d - i2;
        auto dj = j2d - j2;
        auto c1 = blend(p1, p2, 1 - di, di);
        auto c2 = blend(p3, p4, 1 - di, di);
        auto orig = blend(c1, c2, 1 - dj, dj);
#else
        auto orig = src(i2, j2);
        auto p1 = orig;
#endif
        if (zoom_eq)
          dest(i, j) = orig;
        else if (zoom_out)
          dest(i, j) = with_extra(orig, 20);
        else {
          auto ex = extra(orig) + 1; // Ensure result is overdrawn
          if (ex > 20)
            ex = 20;
          dest(i, j) = with_extra(orig, ex);
        }
      } else
        dest(i, j) = grey;
    }
}

void fractals::AsyncRenderer::remap_viewport(Viewport &vp, double dx, double dy,
                                             double r) const {

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

  Viewport dest;
  dest.width = vp.width;
  dest.height = vp.height;
  dest.data = new_contents.data();
  map_viewport(vp, dest, dx, dy, r);
  std::copy(new_contents.begin(), new_contents.end(), vp.data);
}

void fractals::AsyncRenderer::redraw(Viewport &vp) {
  stop_current_calculation();
  Renderer::redraw(vp);
}

void fractals::AsyncRenderer::set_aspect_ratio(int new_width, int new_height) {
  stop_current_calculation();
}

bool fractals::AsyncRenderer::set_coords(const view_coords &c, Viewport &vp) {
  redraw(vp);
  coords = c; // TODO: Update aspect ratio
  return true;
}

void fractals::AsyncRenderer::scroll(int dx, int dy, Viewport &vp) {
  stop_current_calculation();

  // TODO: Only recalculate necessary regions
  coords = coords.scroll(vp.width, vp.height, dx, dy);

  remap_viewport(vp, dx, dy, 1.0);

  vp.updated();
}

void fractals::AsyncRenderer::enable_auto_depth(bool value) {
  automaticallyAdjustDepth = value;
}

void fractals::AsyncRenderer::set_threading(int threads) {
  this->threads = threads;
}

void fractals::AsyncRenderer::get_depth_range(double &min, double &p,
                                              double &max) {
  stop_current_calculation(); // ?? Why
  min = metrics.min_depth;
  max = metrics.max_depth;
  p = metrics.discovered_depth;
}

fractals::AsyncRenderer::my_rendering_sequence::my_rendering_sequence(
    const PointwiseCalculation &calculation, const ColourMap &cm, Viewport &vp,
    std::vector<depth_value> &depths)
    : fractals::buffered_rendering_sequence<double>(vp.width, vp.height, 16),
      calculation(calculation), cm(cm), vp(vp), depths(depths) {
  depths.clear();
}

void fractals::AsyncRenderer::my_rendering_sequence::layer_complete(
    int stride) {
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
      }
#if 1 // Useful to be able to disable this for debugging
      if (stride > 1) {
        // Interpolate the region
        if (x > 0 && y > 0) {
          maybe_fill_region(vp, x - stride, y - stride, x, y);
        }

        auto d = stride / 2;
        int x0 = x - d;
        int x1 = x + d;
        int y0 = y - d;
        int y1 = y + d;
        if (x0 < 0)
          x0 = 0;
        if (x1 >= vp.width)
          x1 = vp.width - 1;
        if (y0 < 0)
          y0 = 0;
        if (y1 >= vp.height)
          y1 = vp.height - 1;
        interpolate_region(vp, x, y, x0, y0, x1, y1);
      }
#endif
    } else {
      // Already calculated which is good
    }
  }

  vp.updated();
}

double fractals::AsyncRenderer::my_rendering_sequence::get_point(int x, int y) {
  if (extra(vp(x, y)) == 0)
    return std::numeric_limits<double>::quiet_NaN();
  ++calculated_pixels;
  auto depth = calculation.calculate(x, y);
  if (depth > 0) {
    // Technically this is a race condition
    // but we want to capture this here (and not in layer_complete())
    // because we need this in case we abort computation before the first layer
    // is complete.
    if (depth < min_depth || min_depth == 0)
      min_depth = depth;
    if (depth > max_depth)
      max_depth = depth;
  }
  return depth;
}