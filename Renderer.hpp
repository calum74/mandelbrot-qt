#pragma once
#include <atomic>
#include <memory>

namespace fractals {
class Viewport;
class ColourMap;
class view_coords;
class PointwiseFractal;

class Renderer {
public:
  virtual ~Renderer() = default;

  // Calculate the entire fractal, or return early if `stop` becomes
  // true. Call vp one or more times to update the view.
  virtual void calculate(Viewport &vp, const ColourMap &cm,
                         std::atomic<bool> &stop);

  // Perform any expensive one-off initialization like
  // computing an orbit.
  virtual void start_async_calculation(Viewport &vp,
                                       std::atomic<bool> &stop) = 0;
  // Calculate a single point
  // Returns the depth, potentially smoothed
  virtual double calculate_point(int w, int h, int x, int y) = 0;

  // Also update the viewport and interpolates it so it can be rendered
  // straight away.
  // returns false if the action is invalid for the current fractal, for example
  // if the resolution is no longer suitable. If false is returned, then the
  // current view is unchanged. Can also prevent zooming out.
  virtual bool zoom(double r, int cx, int cy, Viewport &vp) = 0;
  virtual void scroll(int dx, int dy, Viewport &vp) = 0;

  // !! This should be called "resize(Viewport & old, Viewport & new)"
  // Extends or contracts the current size such that the scale and top-left of
  // the fractal remains the same.
  virtual void set_aspect_ratio(Viewport &vp) = 0;

  // Get an approximate width of the fractal for display purposes.
  virtual double width() const = 0;

  // virtual void reset(Viewport &vp) = 0;

  // Returns immediately, and vp will get called back at some time in the
  // future.
  virtual void calculate_async(Viewport &vp, const ColourMap &cm);

  // Also marks the relevant pixels to redraw in the viewport
  virtual void increase_iterations(Viewport &vp);

  // Also marks the relevant pixels to redraw in the viewport
  virtual void decrease_iterations(Viewport &vp);

  virtual view_coords get_coords() const = 0;

  // Returns true on success (and value changed)
  // False on failure (coords not changed).
  // Failure could be due to incorrect precision for the fractal.
  // May invalidate the viewport
  virtual bool set_coords(const view_coords &w, Viewport &vp) = 0;

  virtual int iterations() const = 0;

  virtual void redraw(Viewport &vp);

  virtual view_coords initial_coords() const = 0;

  virtual void set_fractal(const PointwiseFractal &fractal);

  virtual double get_average_iterations() const;

  virtual double get_average_skipped_iterations() const;
};

std::unique_ptr<Renderer> make_renderer();
} // namespace fractals