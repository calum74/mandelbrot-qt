#include "AsyncRenderer.hpp"

#include "ColourMap.hpp"
#include "Viewport.hpp"
#include "number_cast.hpp"
#include "fractal_calculation.hpp"
#include "high_exponent_real.hpp"
#include "percentile.hpp"
#include "registry.hpp"
#include "view_parameters.hpp"
#include <cassert>

fractals::AsyncRenderer::AsyncRenderer(const fractal &fractal,
                                       Registry &registry)
    : current_fractal(fractal.create()), registry(registry) {
  coords = initial_coords();
}

fractals::AsyncRenderer::~AsyncRenderer() { stop_current_calculation(); }

void fractals::AsyncRenderer::load(const view_parameters &params,
                                   Viewport &vp) {
  stop_current_calculation();

  coords = params;
  auto new_fractal = registry.lookup(params.algorithm);

  if (new_fractal)
    current_fractal = new_fractal->create();

  redraw(vp);
}

void fractals::AsyncRenderer::save(view_parameters &params) const {
  coords.write(params);
  params.algorithm = current_fractal->name();
}

void fractals::AsyncRenderer::increase_iterations(Viewport &vp) {
  stop_current_calculation();
  for (int j = 0; j < vp.height(); ++j)
    for (int i = 0; i < vp.width(); ++i) {
      auto &c = vp(i, j);
      if (c.value>0) {
        // Only redraw final coloured points
        c.error = 127;
      }
    }
  coords.max_iterations *= 2;
}

void fractals::AsyncRenderer::decrease_iterations(Viewport &vp) {
  stop_current_calculation();
  vp.invalidateAllPixels();
  coords.max_iterations /= 2;
}

double fractals::AsyncRenderer::get_average_iterations() const {
  return calculation->average_iterations();
}

double fractals::AsyncRenderer::get_average_skipped_iterations() const {
  return calculation->average_skipped();
}

void fractals::AsyncRenderer::discovered_depth(
    const calculation_metrics &metrics) {
  if (!metrics.last_action_was_a_scroll && metrics.non_black_points > 100 &&
      metrics.discovered_depth > 250 || (metrics.discovered_depth*2 > coords.max_iterations)) {                     // Fudge factor
    coords.max_iterations = metrics.discovered_depth * 2; // Fudge factor
  }
}

void fractals::AsyncRenderer::set_fractal(const fractals::fractal &f) {
  stop_current_calculation();
  current_fractal = f.create();
}

std::string fractals::AsyncRenderer::get_fractal_family() const {
  return current_fractal->family();
}

std::string fractals::AsyncRenderer::get_fractal_name() const {
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

void fractals::AsyncRenderer::calculate_in_thread(fractals::Viewport &vp,
                                                  std::atomic<bool> &stop) {

  my_rendering_sequence calculated_pixels(*calculation, vp);
  calculated_pixels.calculate(threads, stop);

  // Find the depths.
  std::vector<int> coloured_pixels;
  coloured_pixels.reserve(vp.values.size());
  for (int i = 0; i < vp.values.size(); ++i) {
    if (vp.values[i].error == 0 && vp.values[i].value > 0)
      coloured_pixels.push_back(i);
  }

  metrics.non_black_points = coloured_pixels.size();

#if 1 
  std::cout << "Finished calculation\n";
  std::cout << "  We calculated " << calculated_pixels.points_calculated
  << " points\n";

  std::cout << "  We have " << coloured_pixels.size() << " coloured pixels\n";
#endif

  if (coloured_pixels.size() > 100) {
    // ?? Why are we doing this? Can't we just take the top pixel and be done
    // with it?
    auto cmp = [&](int a, int b) {
      return vp.values[a].value < vp.values[b].value;
    };
    auto p999 = *top_percentile(coloured_pixels.begin(), coloured_pixels.end(),
                                0.999, cmp);
    auto p9999 = *top_percentile(coloured_pixels.begin(), coloured_pixels.end(),
                                 0.9999, cmp);
    metrics.p999 = vp.values[p999].value;
    metrics.p9999 = vp.values[p9999].value;
    metrics.p9999_x = p9999 % vp.values.width();
    metrics.p9999_y = p9999 / vp.values.width();
    // std::cout << "  p99.9 = " << metrics.p999 << " p99.99 = " << metrics.p9999
    //          << " at " << metrics.p9999_x << ", " << metrics.p9999_y << "\n";
  }

  metrics.min_depth = calculated_pixels.min_depth;
  metrics.max_depth = calculated_pixels.max_depth;
  metrics.points_calculated = calculated_pixels.points_calculated;
}

void fractals::AsyncRenderer::calculate_async(fractals::Viewport &view) {
  stop_current_calculation();

  stop = false;

  current_calculation = std::async([&]() {
    t0 = std::chrono::high_resolution_clock::now();

    calculation =
        current_fractal->create(coords, view.width(), view.height(), stop);

    metrics.min_depth = 0;
    metrics.max_depth = 0;
    metrics.discovered_depth = 0;
    metrics.p999 = 0;
    metrics.p9999 = 0;
    calculate_in_thread(view, stop);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (metrics.p999 > 0) {
      metrics.discovered_depth = metrics.p999;
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

  auto new_coords = lockCenter
                        ? coords.zoom(r)
                        : coords.zoom(r, vp.width(), vp.height(), cx, cy);

  if (!current_fractal->valid_for(new_coords)) {
    return false;
  }

  coords = new_coords;

  remap_viewport(vp, cx * (1 - r), cy * (1 - r), r);

  metrics.last_action_was_a_scroll = false;
  vp.updated();
  return true;
}

fractals::view_coords fractals::AsyncRenderer::get_coords() const {
  return coords;
}

double fractals::AsyncRenderer::log_width() const {
  return fractals::log(number_cast<high_exponent_real<>>(coords.r));
}

int fractals::AsyncRenderer::iterations() const {
  return coords.max_iterations;
}

void fractals::AsyncRenderer::center(Viewport &vp) {
  if (max_x > 0 && max_y > 0)
    scroll(max_x - vp.width() / 2, max_y - vp.height() / 2, vp);
}

void fractals::AsyncRenderer::zoom_in(Viewport &vp) {
  zoom(0.5, vp.width() / 2, vp.height() / 2, true, vp);
}

bool fractals::AsyncRenderer::get_auto_zoom(int &x, int &y) {
  if (metrics.p9999 > 0) {
    x = metrics.p9999_x;
    y = metrics.p9999_y;
    return true;
  }
  return false;
}

void fractals::AsyncRenderer::remap_viewport(Viewport &vp, double dx, double dy,
                                             double r) const {
  Viewport dest;
  dest.init(vp.width(), vp.height());
  map_viewport(vp, dest, dx, dy, r);

  vp = std::move(dest);
}

void fractals::AsyncRenderer::redraw(Viewport &vp) {
  stop_current_calculation();
  Renderer::redraw(vp);
}

void fractals::AsyncRenderer::set_aspect_ratio(int new_width, int new_height) {
  stop_current_calculation();
}

bool fractals::AsyncRenderer::set_coords(const view_coords &c, Viewport &vp) {
  metrics.last_action_was_a_scroll = false;
  redraw(vp);
  coords = c; // TODO: Update aspect ratio
  return true;
}

void fractals::AsyncRenderer::scroll(int dx, int dy, Viewport &vp) {
  stop_current_calculation();

  // TODO: Only recalculate necessary regions
  coords = coords.scroll(vp.width(), vp.height(), dx, dy);

  remap_viewport(vp, dx, dy, 1.0);

  metrics.last_action_was_a_scroll = true;
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
    fractal_calculation &calculation, Viewport &vp)
    : vp(vp), calculation_pixmap(vp.values, 16, calculation) {}

void fractals::AsyncRenderer::my_rendering_sequence::layer_complete(
    int stride) {
  // Transfer and interpolate to the current viewport
  //map_pixmap(pixels, vp.values, [&](auto c) {
  //  return c;
  //});
  vp.updated();
}

fractals::mapped_point
fractals::AsyncRenderer::map_point(const Viewport &vp,
                                   const view_coords &c) const {

  return coords.map_point(vp.width(), vp.height(), c);
}
