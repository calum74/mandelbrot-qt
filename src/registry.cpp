#include "registry.hpp"
#include "fractal.hpp"

#include <string>

namespace {
using namespace fractals;

class RegistryImpl : public Registry {
  void add(const pointwise_fractal &f) override {
    fractals.push_back(
        std::pair<std::string, const pointwise_fractal &>{f.name(), f});
  }

  std::vector<std::pair<std::string, const fractals::pointwise_fractal &>>
      fractals;

  const pointwise_fractal *lookup(const std::string &query) const override {
    if (fractals.empty())
      return {};
    // !! Linear search
    for (auto &[name, fractal] : fractals) {
      if (name == query)
        return &fractal;
    }
    return &fractals.front().second;
  }

  std::vector<std::pair<std::string, const fractals::pointwise_fractal &>>
  listFractals() const override {
    return fractals;
  }
};
} // namespace

std::unique_ptr<fractals::Registry> fractals::make_registry() {
  return std::make_unique<RegistryImpl>();
}
