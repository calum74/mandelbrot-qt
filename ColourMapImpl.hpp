#include "ColourMap.hpp"
#include "fractal.hpp"
#include <random>
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  std::vector<RGB> colours;
  double gamma = 0.8, k = 0.1, offset = 0.0;
  std::default_random_engine e;

  ColourMapImpl();
  RGB operator()(double d) const override;
  void randomize() override;
  void setRange(double min, double max) override;
};
} // namespace fractals