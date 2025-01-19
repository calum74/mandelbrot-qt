#include "circle.hpp"
#include "mandelbrot.hpp"
#include "registry.hpp"

void register_fractals(fractals::Registry &r) {
  r.add(mandelbrot_fractal);
  r.add(mandelbrot3_fractal);
  r.add(mandelbrot4_fractal);
  r.add(mandelbrot5_fractal);
  r.add(mandelbrot6_fractal);
  r.add(mandelbrot7_fractal);
  r.add(mandeldrop_fractal);

  // To add custom fractals, insert them into this list, for example:
  // r.add(circle);
  // r.add(experimental_fractal);
}