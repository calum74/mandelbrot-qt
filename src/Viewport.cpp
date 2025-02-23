#include "Viewport.hpp"
#include <cassert>

constexpr fractals::error_value<double> missing_value = {
    std::numeric_limits<double>::quiet_NaN(), 127};

fractals::Viewport::iterator fractals::Viewport::begin() {
  return values.begin();
}

fractals::Viewport::iterator fractals::Viewport::end() { return values.end(); }

int fractals::Viewport::size() const { return values.size(); }

void fractals::Viewport::calculation_started(double log_radius,
                                             int iterations) {}

void fractals::Viewport::schedule_next_calculation() {}

void fractals::Viewport::start_timer() {}

void fractals::Viewport::stop_timer() {}

void fractals::Viewport::invalidateAllPixels() {
  for (auto &p : values) {
    p.error = 127;
  }
}

void fractals::Viewport::init(int w0, int h0) {
  values = {w0, h0, missing_value};
}

void fractals::map_viewport(const Viewport &src, Viewport &dest, double dx,
                            double dy, double r) {

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  map_pixmap(
      src.values, dest.values, dx, dy, r,
      [&](error_value<double> p) -> error_value<double> {
        if (zoom_eq)
          return p;
        if (zoom_out)
          return {p.value, 20}; // When zooming out, don't keep the old image
                                // as it looks wierd
        std::uint8_t ex = p.error + 1; // Ensure result is overdrawn
        if (ex > 20)
          ex = 20;
        return {p.value, ex};
      },
      missing_value);
}

fractals::error_value<double> fractals::Viewport::invalid_value() const {
  return missing_value;
}
