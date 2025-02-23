#include "Renderer.hpp"

#include "AsyncRenderer.hpp"
#include "Viewport.hpp"
#include "mandelbrot.hpp"
#include <cassert>

void fractals::Renderer::increase_iterations(Viewport &) {}

void fractals::Renderer::decrease_iterations(Viewport &) {}

void fractals::Renderer::center(Viewport &vp) {}

void fractals::Renderer::zoom_in(Viewport &vp) {}

void fractals::Renderer::redraw(Viewport &vp) {
  vp.invalidateAllPixels();
}

void fractals::Renderer::enable_auto_depth(bool value) {}

void fractals::Renderer::set_threading(int threads) {}

void fractals::Renderer::get_depth_range(double &min, double &p, double &max) {}

void fractals::Renderer::calculate_async(fractals::Viewport &view) {}

double fractals::Renderer::get_average_iterations() const { return 0; }

double fractals::Renderer::get_average_skipped_iterations() const { return 0; }

void fractals::Viewport::updated() {}

void fractals::Viewport::finished(const RenderingMetrics &metrics) {}

void fractals::Renderer::discovered_depth(const RenderingMetrics &) {}

// Arguable these should be pure virtual functions
void fractals::Renderer::set_fractal(const fractals::fractal &) {}

void fractals::Renderer::load(const view_parameters &params, Viewport &vp) {}

void fractals::Renderer::save(view_parameters &params) const {}

std::unique_ptr<fractals::Renderer> fractals::make_renderer(Registry &reg) {
  return std::make_unique<AsyncRenderer>(mandelbrot_fractal, reg);
}
