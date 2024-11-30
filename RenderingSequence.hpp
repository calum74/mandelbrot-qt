#pragma once

namespace fractals {
// Visits all points (x,y) in the region of size w*h
class RenderingSequence {
public:
  RenderingSequence(int w, int h, int stride = 128)
      : width(w), height(h), initial_stride(stride), stride(stride), x(0),
        y(0) {}

  const int width, height;
  int x, y; // Current
  const int initial_stride;
  int stride;

  // Visit all points using the stride
  bool next0() {
    x += stride;
    if (x >= width) {
      x = 0;
      y += stride;
      if (y >= height) {
        if (stride == 1)
          return false;
        stride = stride / 2;
        x = 0;
        y = 0;
      }
    }

    return true;
  }

  bool already_done() const {
    if (stride == initial_stride)
      return false;

    // Is the current_point already emitted.
    // Look at the least significant bits
    int mask = (stride * 2) - 1;
    return !(x & mask) && !(y & mask);
  }

  bool next() {
    while (next0()) {
      if (!already_done())
        return true;
    }
    return false;
  }
};

} // namespace fractals
