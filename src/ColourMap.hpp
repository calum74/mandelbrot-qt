#pragma once
#include "rgb.hpp"
#include <memory>

namespace fractals {
struct view_parameters;
struct shader_parameters;

class shader {
public:
  virtual ~shader() = default;
  virtual void randomize() = 0;
  virtual void resetGradient() = 0;
  virtual void setRange(double min, double max) = 0;
  virtual void maybeUpdateRange(double min, double max) = 0;
  virtual RGB operator()(double depth, double dx, double dy) const = 0;
  virtual RGB operator()(double depth) const = 0;

  virtual void load(const view_parameters &) = 0;
  virtual void save(view_parameters &) const = 0;
  virtual void setParameters(const shader_parameters &) = 0;
  virtual void getParameters(shader_parameters &params) = 0;
};

std::unique_ptr<shader> make_colourmap();
} // namespace fractals
