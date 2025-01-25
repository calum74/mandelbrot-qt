#pragma once
#include "RGB.hpp"

namespace fractals {
/*
A region to render. It is essentially a buffer of pixels
*/
struct Viewport {
  using value_type = RGB;
  using size_type = int;

  size_type width = 0, height = 0;

  // The buffer to receive the pixels, stored in row-order.
  // data must be width*height elements large.
  value_type *data = 0;

  value_type &operator()(size_type x, size_type y) {
    return data[x + y * width];
  }
  value_type operator()(size_type x, size_type y) const {
    return data[x + y * width];
  }

  using iterator = value_type *;
  using const_iterator = const value_type *;
  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  int size() const; // width * height

  void invalidateAllPixels();

  // Callback when data has changed in the given pixel-region
  virtual void region_updated(int x, int y, int w, int h);

  // Callback when viewport fully calculated.
  // May not get called if cancelled.
  virtual void finished(double log_width, int min_depth, int max_depth,
                        double avg_iterations, double avg_skipped_iterations,
                        double render_time);

  // Can be called even if stopped
  virtual void discovered_depth(int points, double discovered_depth,
                                double seconds_per_point);
};

// Perform a pixel-by-pixel remapping and interpolation from src to dest.
void map_viewport(const Viewport &src, Viewport &dest, double dx, double dy,
                  double r);

} // namespace fractals