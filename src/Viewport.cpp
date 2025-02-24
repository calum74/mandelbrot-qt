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

namespace fractals {
template <typename ErrorFn>
void interpolate_viewport(const Viewport &src, Viewport &dest, double dx,
                          double dy, double r, ErrorFn fn) {

  for (int j = 0; j < dest.height(); ++j)
    for (int i = 0; i < dest.width(); ++i) {
      double rx = r * i + dx, ry = r * j + dy;
      int i2 = rx;
      int j2 = ry;
      auto &to_pixel = dest(i, j);
      if (i2 >= 0 && i2 < dest.width() && j2 >= 0 && j2 < dest.height()) {
        rx -= i2;
        ry -= j2;
        auto &from_pixel_00 = src(i2, j2);
        auto &from_pixel_10 = src(i2 + 1 < dest.width() ? i2 + 1 : i2, j2);
        auto &from_pixel_01 = src(i2, j2 + 1 < dest.height() ? j2 + 1 : j2);
        auto &from_pixel_11 = src(i2 + 1 < dest.width() ? i2 + 1 : i2,
                                  j2 + 1 < dest.height() ? j2 + 1 : j2);

        auto to_value = from_pixel_00.value * (1 - rx) * (1 - ry) +
                        from_pixel_10.value * rx * (1 - ry) +
                        from_pixel_01.value * (1 - rx) * ry +
                        from_pixel_11.value * rx * ry;
        auto to_error =
            std::max(std::max(from_pixel_00.error, from_pixel_10.error),
                     std::max(from_pixel_01.error, from_pixel_11.error));
        to_pixel = {to_value, fn(to_error)};
      } else {
        to_pixel = missing_value;
      }
    }
}
} // namespace fractals

void fractals::map_viewport(const Viewport &src, Viewport &dest, double dx,
                            double dy, double r) {

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  interpolate_viewport(src, dest, dx, dy, r, [&](int e) {
    return zoom_eq ? e : zoom_out ? 20 : e > 20 ? e : e + 1;
  });

#if 0
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
#endif
}

void fractals::interpolate_viewport(const Viewport &src, Viewport &dest,
                                    double dx, double dy, double r) {

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  interpolate_viewport(src, dest, dx, dy, r, [](int e) { return e; });
}

fractals::error_value<double> fractals::Viewport::invalid_value() const {
  return missing_value;
}
