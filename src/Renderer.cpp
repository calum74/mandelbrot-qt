#include "Renderer.hpp"

#include "AsyncRenderer.hpp"
#include "Viewport.hpp"
#include "mandelbrot.hpp"

// !! Not used

namespace fractals {
RGB blend(RGB c1, RGB c2, int w1, int w2) {
  return make_rgb((red(c1) * w1 + red(c2) * w2) / (w1 + w2),
                  (green(c1) * w1 + green(c2) * w2) / (w1 + w2),
                  (blue(c1) * w1 + blue(c2) * w2) / (w1 + w2));
}
} // namespace fractals

void fractals::interpolate_region(Viewport &vp, int x0, int y0, int h) {
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

void fractals::Viewport::region_updated(int x, int y, int w, int h) {}

void fractals::Viewport::discovered_depth(int, double) {}

void fractals::Viewport::finished(double, int, int, double, double, double) {}

void fractals::Renderer::discovered_depth(int, double) {}

void fractals::Renderer::set_fractal(
    const fractals::PointwiseCalculationFactory &) {}

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
