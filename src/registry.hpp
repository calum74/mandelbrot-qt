#pragma once

#include <memory>
#include <string>
#include <vector>

namespace fractals {

class PointwiseCalculationFactory;

class Registry {
public:
  virtual ~Registry() = default;
  virtual void add(const PointwiseCalculationFactory &) = 0;
  virtual std::vector<
      std::pair<std::string, const fractals::PointwiseCalculationFactory &>>
  listFractals() const = 0;

  virtual const PointwiseCalculationFactory *
  lookup(const std::string &name) const = 0;
};

// Perhaps this isn't useful
std::unique_ptr<Registry> make_registry();

} // namespace fractals