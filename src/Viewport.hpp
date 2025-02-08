#pragma once
#include "RGB.hpp"
#include <vector>

namespace fractals {
struct RenderingMetrics;

/*
A region to render. It is essentially a buffer of pixels
*/
class Viewport {
public:
  using value_type = RGB;
  using size_type = int;

  int width() const { return w; }

  int height() const { return h; }

  // The buffer to receive the pixels, stored in row-order.
  // data must be width*height elements large.
  value_type *data = 0;
  std::vector<std::uint8_t> error_data;

  void init(int w, int h, value_type *data);

  value_type &operator()(size_type x, size_type y) { return data[x + y * w]; }

  const value_type &operator()(size_type x, size_type y) const {
    return data[x + y * w];
  }

  // Get/set the error corresponding to a value
  std::uint8_t &error(value_type &x) { return error_data[&x - data]; }
  std::uint8_t error(const value_type *x) const { return error_data[x - data]; }

  using iterator = value_type *;
  using const_iterator = const value_type *;
  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  int size() const; // width * height

  void invalidateAllPixels();

  // Callback when data has changed and it's time to render
  virtual void updated();

  // Callback when calculation has completed
  virtual void finished(const RenderingMetrics &metrics);

  // Signal that a new calculation has started
  virtual void calculation_started(double log_radius, int iterations);

  // The current animation has finished, so queue an async callback
  // to carry on animating
  virtual void schedule_next_calculation();

  // Schedule the next frame to render
  virtual void start_timer();
  virtual void stop_timer();

private:
  int w = 0, h = 0;
};

// Perform a pixel-by-pixel remapping and interpolation from src to dest.
void map_viewport(const Viewport &src, Viewport &dest, double dx, double dy,
                  double r);

} // namespace fractals