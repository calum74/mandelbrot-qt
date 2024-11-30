#pragma once
#include "RGB.hpp"
#include <memory>

namespace fractals {
class ColourMap {
public:
  virtual ~ColourMap() = default;
  virtual void randomize() = 0;
  virtual void setRange(double min, double max) = 0;
  virtual RGB operator()(double f) const = 0;
};

std::unique_ptr<ColourMap> make_colourmap();
} // namespace fractals
