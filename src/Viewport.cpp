#include "Viewport.hpp"
#include <cassert>

constexpr fractals::Viewport::value_type grey = {
    fractals::make_rgb(100, 100, 100), 127};

fractals::Viewport::iterator fractals::Viewport::begin() {
  return pixels.begin();
}

fractals::Viewport::iterator fractals::Viewport::end() { return pixels.end(); }

int fractals::Viewport::size() const { return pixels.size(); }

void fractals::Viewport::calculation_started(double log_radius,
                                             int iterations) {}

void fractals::Viewport::schedule_next_calculation() {}

void fractals::Viewport::start_timer() {}

void fractals::Viewport::stop_timer() {}

void fractals::Viewport::invalidateAllPixels() {
  for (auto &p : pixels) {
    p.error = 127;
  }
}

void fractals::Viewport::init(int w0, int h0) { pixels = {w0, h0, grey}; }

void fractals::map_viewport(const Viewport &src, Viewport &dest, double dx,
                            double dy, double r) {

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  map_pixmap(
      src.pixels, dest.pixels, dx, dy, r,
      [&](Viewport::value_type p) -> Viewport::value_type {
        if (zoom_eq)
          return p;
        if (zoom_out)
          return {p.colour, 20};  // When zooming out, don't keep the old image as it looks wierd
        std::uint8_t ex = p.error + 1; // Ensure result is overdrawn
        if (ex > 20)
          ex = 20;
        return {p.colour, ex};
      },
      grey);
}
