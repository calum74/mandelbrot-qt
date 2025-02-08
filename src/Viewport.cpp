#include "Viewport.hpp"
#include <cassert>

constexpr fractals::RGB grey = fractals::make_rgb(100, 100, 100);

fractals::Viewport::iterator fractals::Viewport::begin() { return data; }

fractals::Viewport::iterator fractals::Viewport::end() { return data + size(); }

int fractals::Viewport::size() const { return width * height; }

void fractals::Viewport::calculation_started(double log_radius,
                                             int iterations) {}

void fractals::Viewport::schedule_next_calculation() {}

void fractals::Viewport::start_timer() {}

void fractals::Viewport::stop_timer() {}

void fractals::Viewport::invalidateAllPixels() {
  for (auto j = 0; j < height; ++j)
    for (auto i = 0; i < width; ++i) {
      error((*this)(i, j)) = 127;
    }
}

void fractals::Viewport::init(int w, int h, RGB *d) {
  width = w;
  height = h;
  data = d;
  error_data.resize(w * h);
  invalidateAllPixels();
}

void fractals::map_viewport(const Viewport &src, Viewport &dest, double dx,
                            double dy, double r) {
  // One day, we might be able to remap to a different size
  assert(src.width == dest.width);
  assert(src.height == dest.height);

  bool zoom_eq = r == 1.0;
  bool zoom_out = r > 1.0;

  for (int j = 0; j < dest.height; ++j)
    for (int i = 0; i < dest.width; ++i) {
      int i2 = r * i + dx;
      int j2 = r * j + dy;
      auto &to_pixel = dest(i, j);
      // int i2 = i2d;
      // int j2 = j2d;
      if (i2 >= 0 && i2 < dest.width && j2 >= 0 && j2 < dest.height) {
#if 0
        // This is so slow!!!
        auto i22 = i2 < dest.width - 1 ? i2 + 1 : i2;
        auto j22 = j2 < dest.height - 1 ? j2 + 1 : j2;
        auto p1 = src(i2, j2);
        auto p2 = src(i22, j2);
        auto p3 = src(i2, j22);
        auto p4 = src(i22, j22);
        auto di = i2d - i2;
        auto dj = j2d - j2;
        auto c1 = blend(p1, p2, 1 - di, di);
        auto c2 = blend(p3, p4, 1 - di, di);
        auto orig = blend(c1, c2, 1 - dj, dj);
#else
        auto &from_pixel = src(i2, j2);
#endif
        to_pixel = from_pixel;
        if (zoom_eq)
          dest.error(to_pixel) = src.error(&from_pixel);
        else if (zoom_out)
          dest.error(to_pixel) = 20;
        else {
          auto ex = src.error(&from_pixel) + 1; // Ensure result is overdrawn
          if (ex > 20)
            ex = 20;
          dest.error(to_pixel) = ex;
        }
      } else {
        to_pixel = grey;
        dest.error(to_pixel) = 127;
      }
    }
}
