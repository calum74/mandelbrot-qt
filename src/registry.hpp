#pragma once

#include <memory>
#include <string>
#include <vector>

namespace fractals {

class pointwise_fractal;

class Registry {
public:
  virtual ~Registry() = default;
  virtual void add(const pointwise_fractal &) = 0;
  virtual std::vector<
      std::pair<std::string, const fractals::pointwise_fractal &>>
  listFractals() const = 0;

  virtual const pointwise_fractal *lookup(const std::string &name) const = 0;
};

// Perhaps this isn't useful
std::unique_ptr<Registry> make_registry();

} // namespace fractals