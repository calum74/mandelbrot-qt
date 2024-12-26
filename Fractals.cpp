#include "circle.hpp"
#include "mandelbrot.hpp"

void register_fractals(fractals::Registry &r) {
  r.add(mandelbrot_fractal);
  r.add(cubic_mandelbrot_fractal);
  r.add(mandeldrop_fractal);

  // To add custom fractals, insert them into this list, for example:
  // r.add(circle);
}