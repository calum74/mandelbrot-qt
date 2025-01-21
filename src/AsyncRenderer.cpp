#include "AsyncRenderer.hpp"

fractals::AsyncRenderer::AsyncRenderer(const PointwiseFractal &fractal,
                                       Registry &registry)
    : current_fractal(&fractal), registry(registry) {
  coords = initial_coords();
}

fractals::AsyncRenderer::~AsyncRenderer() { stop_current_calculation(); }

void fractals::AsyncRenderer::load(const view_parameters &params,
                                   Viewport &vp) {
  stop_current_calculation();

  coords = params.coords;
  auto new_fractal = registry.lookup(params.fractal_name);

  if (new_fractal)
    current_fractal = new_fractal;

  redraw(vp);
}

void fractals::AsyncRenderer::save(view_parameters &params) const {
  params.coords = coords;
  params.fractal_name = current_fractal->name();
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