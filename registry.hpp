#pragma once

#include <string>
#include <vector>

namespace fractals {

class PointwiseFractal;

class Registry {
public:
  virtual ~Registry() = default;
  virtual void add(const PointwiseFractal &) = 0;
  virtual std::vector<
      std::pair<std::string, const fractals::PointwiseFractal &>>
  listFractals() const = 0;
};

// Perhaps this isn't useful
std::unique_ptr<Registry> make_registry();

} // namespace fractals