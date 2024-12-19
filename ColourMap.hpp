#pragma once
#include "RGB.hpp"

namespace fractals {
class ColourMap {
public:
  virtual ~ColourMap() = default;
  virtual void randomize() = 0;
  virtual void setRange(double min, double max) = 0;
  virtual RGB operator()(double f) const = 0;
};
} // namespace fractals
