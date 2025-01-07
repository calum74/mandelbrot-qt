#pragma once
#include "RGB.hpp"
#include <memory>

namespace fractals {
class view_parameters;

class ColourMap {
public:
  virtual ~ColourMap() = default;
  virtual void randomize() = 0;
  virtual void setRange(double min, double max) = 0;
  virtual RGB operator()(double f) const = 0;

  virtual void load(const view_parameters &) = 0;
  virtual void save(view_parameters &) const = 0;
};

std::unique_ptr<ColourMap> make_colourmap();
} // namespace fractals
