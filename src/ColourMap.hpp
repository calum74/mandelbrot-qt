#pragma once
#include "RGB.hpp"
#include <memory>

namespace fractals {
struct view_parameters;

class ColourMap {
public:
  virtual ~ColourMap() = default;
  virtual void randomize() = 0;
  virtual void resetGradient() = 0;
  virtual void setRange(double min, double max) = 0;
  virtual void maybeUpdateRange(double min, double max) = 0;
  virtual void enableAutoGradient() = 0;
  virtual void disableAutoGradient() = 0;
  virtual void enableShading() = 0;
  virtual void disableShading() = 0;
  virtual RGB operator()(double depth, double dx, double dy) const = 0;
  virtual RGB operator()(double depth) const = 0;

  virtual void load(const view_parameters &) = 0;
  virtual void save(view_parameters &) const = 0;

  virtual void setSeed(int seed) = 0;
  virtual int getSeed() const = 0;
  virtual void setGradient(double gradient) = 0;
  virtual double getGradient() const = 0;
  virtual void setOffset(double offset) = 0;
  virtual double getOffset() const = 0;
};

std::unique_ptr<ColourMap> make_colourmap();
} // namespace fractals
