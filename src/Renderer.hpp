#pragma once
#include <atomic>
#include <iostream>
#include <memory>

namespace fractals {
class Viewport;
class ColourMap;
class view_coords;
class PointwiseFractal;
struct RenderingMetrics;
class Registry;
struct view_parameters;

class Renderer {
public:
  virtual ~Renderer() = default;

  // Also update the viewport and interpolates it so it can be rendered
  // straight away.
  // returns false if the action is invalid for the current fractal, for example
  // if the resolution is no longer suitable. If false is returned, then the
  // current view is unchanged. Can also prevent zooming out.
  virtual bool zoom(double r, int cx, int cy, bool lockCenter,
                    Viewport &vp) = 0;
  virtual void scroll(int dx, int dy, Viewport &vp) = 0;

  // !! This should be called "resize(Viewport & old, Viewport & new)"
  // Extends or contracts the current size such that the scale and top-left of
  // the fractal remains the same.
  virtual void set_aspect_ratio(int new_width, int new_height) = 0;

  // Get an approximate (log of) width of the fractal for display purposes.
  virtual double log_width() const = 0;

  // Returns immediately, and vp will get called back at some time in the
  // future.
  virtual void calculate_async(Viewport &vp, const ColourMap &cm);

  // Also marks the relevant pixels to redraw in the viewport
  virtual void increase_iterations(Viewport &vp);

  // Also marks the relevant pixels to redraw in the viewport
  virtual void decrease_iterations(Viewport &vp);

  virtual view_coords get_coords() const = 0;

  virtual void discovered_depth(const RenderingMetrics &metrics);

  // Returns true on success (and value changed)
  // False on failure (coords not changed).
  // Failure could be due to incorrect precision for the fractal.
  // May invalidate the viewport
  virtual bool set_coords(const view_coords &w, Viewport &vp) = 0;

  virtual int iterations() const = 0;

  virtual void redraw(Viewport &vp);

  virtual view_coords initial_coords() const = 0;

  virtual void set_fractal(const PointwiseFractal &fractal) = 0;
  virtual const char *get_fractal_family() const = 0;
  virtual const char *get_fractal_name() const = 0;

  virtual double get_average_iterations() const;
  virtual double get_average_skipped_iterations() const;
  virtual void enable_auto_depth(bool value);
  virtual void set_threading(int threads);
  virtual void get_depth_range(double &min, double &percentile, double &max);
  virtual void save(view_parameters &params) const;
  virtual void load(const view_parameters &params, Viewport &vp);
  virtual void center(Viewport &vp);
  virtual void zoom_in(Viewport &vp);
  virtual bool get_auto_zoom(int &x, int &y) = 0;
};

std::unique_ptr<Renderer> make_renderer(Registry &);
} // namespace fractals