#include "Renderer.hpp"

#include "AsyncRenderer.hpp"
#include "Viewport.hpp"
#include "mandelbrot.hpp"

// !! Not used

bool fractals::maybe_fill_region(Viewport &vp, int x0, int y0, int x1, int y1) {
  if (x1 - x0 > 2)
    return false;
  auto c00 = vp(x0, y0);
  auto c10 = vp(x1, y0);
  auto c01 = vp(x0, y1);
  auto c11 = vp(x1, y1);
  // auto average = blend(blend(c00, c10, 1, 1), blend(c01, c11, 1, 1), 1, 1);

  // If all 4 corners have the same colour, claim that the filled in colour is
  // accurate and does not need to be recalculated 1 means more speed 0 means
  // more accuracy
  if (c00 == c10 && c00 == c11 && c00 == c01) {
    for (int j = y0; j < y1; ++j)
      for (int i = x0; i < x1; ++i)
        vp(i, j) = c00;
    return true;
  }
  return false;
}

void fractals::interpolate_region(Viewport &vp, int cx, int cy, int x0, int y0,
                                  int x1, int y1) {
  auto c = vp(cx, cy);
  assert(x0 >= 0);
  assert(x1 < vp.width);
  assert(y0 >= 0);
  assert(y0 < vp.height);

  // Solid colour
  for (int j = y0; j < y1; ++j) {
    int dy = j - cy;
    int ey = dy < 0 ? -dy : dy;
    for (int i = x0; i < x1; ++i) {
      int dx = i - cx;
      int ex = dx < 0 ? -dx : dx;
      int error = ex + ey;
      auto &p = vp(i, j);
      if (error < extra(p))
        p = with_extra(c, error);
    }
  }
}

void fractals::Renderer::increase_iterations(Viewport &) {}

void fractals::Renderer::decrease_iterations(Viewport &) {}

void fractals::Renderer::center(Viewport &vp) {}

void fractals::Renderer::zoom_in(Viewport &vp) {}

void fractals::Renderer::redraw(Viewport &vp) {
  for (int j = 0; j < vp.height; ++j)
    for (int i = 0; i < vp.width; ++i)
      vp(i, j) = with_extra(vp(i, j), 127);
}

void fractals::Renderer::enable_auto_depth(bool value) {}

void fractals::Renderer::set_threading(int threads) {}

void fractals::Renderer::get_depth_range(double &min, double &p, double &max) {}

void fractals::Renderer::calculate_async(fractals::Viewport &view,
                                         const ColourMap &cm) {}

double fractals::Renderer::get_average_iterations() const { return 0; }

double fractals::Renderer::get_average_skipped_iterations() const { return 0; }

void fractals::Viewport::updated() {}

void fractals::Viewport::discovered_depth(int, double, double, int, int, int) {}

void fractals::Viewport::finished(double, int, int, double, double, double) {}

void fractals::Renderer::discovered_depth(int, double, int, int, int) {}

// Arguable these should be pure virtual functions
void fractals::Renderer::set_fractal(const fractals::PointwiseFractal &) {}

void fractals::Viewport::invalidateAllPixels() {
  for (auto j = 0; j < height; ++j)
    for (auto i = 0; i < width; ++i) {
      (*this)(i, j) = with_extra((*this)(i, j), 127);
    }
}

void fractals::Renderer::load(const view_parameters &params, Viewport &vp) {}

void fractals::Renderer::save(view_parameters &params) const {}

std::unique_ptr<fractals::Renderer> fractals::make_renderer(Registry &reg) {
  return std::make_unique<AsyncRenderer>(mandelbrot_fractal, reg);
}
