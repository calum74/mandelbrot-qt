#pragma once
#include "RGB.hpp"

namespace fractals {
/*
A region to render. It is essentially a buffer of pixels
*/
struct Viewport {
  int width=0, height=0;

  // The buffer to receive the pixels, stored in row-order.
  // data must be width*height elements large.
  RGB *data=0;

  RGB &operator()(int x, int y) { return data[x + y * width]; }
  RGB operator()(int x, int y) const { return data[x + y * width]; }

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
  virtual void discovered_depth(int points, double discovered_depth);
};

} // namespace fractals