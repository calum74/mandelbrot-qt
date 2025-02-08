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
  using size_type = int;

  size_type width() const { return w; }

  size_type height() const { return h; }

  void init(size_type w, size_type h);

  struct pixel
  {
    std::uint32_t colour:24;
    std::uint8_t error:8;

    static constexpr int max_error = 127;
  };

  using value_type = pixel;

  value_type &operator()(size_type x, size_type y) { return pixels[x + y * w]; }

  value_type & operator[](size_type x) { return pixels[x]; }

  const value_type &operator()(size_type x, size_type y) const {
    return pixels[x + y * w];
  }

  using iterator = value_type *;
  using const_iterator = const value_type *;
  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  size_type size() const; // width * height

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
  std::vector<value_type> pixels;
};

// Perform a pixel-by-pixel remapping and interpolation from src to dest.
void map_viewport(const Viewport &src, Viewport &dest, double dx, double dy,
                  double r);

} // namespace fractals