#pragma once
#include "pixmap.hpp"

namespace fractals {
struct RenderingMetrics;

/*
A region to render. It is essentially a buffer of pixels
*/
class Viewport {
public:
  using size_type = int;

  size_type width() const { return values.width(); }

  size_type height() const { return values.height(); }

  void init(size_type w, size_type h);

  using value_type = error_value<double>;

  value_type & operator[](size_type x) { return values[x]; }

  value_type &operator()(size_type x, size_type y) {
    return values(x,y);
  }

  const value_type &operator()(size_type x, size_type y) const {
    return values(x,y);
  }

  using iterator = typename pixmap<value_type>::iterator;
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

  pixmap<value_type> values;

  value_type invalid_value() const;
};

// Perform a pixel-by-pixel remapping and interpolation from src to dest.
void map_viewport(const Viewport &src, Viewport &dest, double dx, double dy,
                  double r);

// Perform a pixel-by-pixel remapping, interpolate values and don't increase the error values
void interpolate_viewport(const Viewport &src, Viewport &dest, double dx, double dy,
  double r);


} // namespace fractals