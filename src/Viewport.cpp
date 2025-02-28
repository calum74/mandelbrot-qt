#include "Viewport.hpp"
#include "view.hpp"
#include <cassert>
#include <limits>

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

void fractals::interpolate_viewport(const Viewport &src, Viewport &dest,
                                    double dx, double dy, double r) {
  interpolate_values(src.values, dest.values, dx, dy, r);
}

void fractals::map_viewport(const Viewport &src, Viewport &dest,
  double dx, double dy, double r) {
  map_values(src.values, dest.values, dx, dy, r);
}


fractals::error_value<double> fractals::Viewport::invalid_value() const {
  return missing_value;
}
