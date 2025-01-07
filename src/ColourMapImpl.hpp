#include "ColourMap.hpp"
#include "fractal.hpp"
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  std::vector<RGB> colours;
  double gamma = 0.8, k = 0.1, offset = 0.0;
  unsigned int seed;

  ColourMapImpl();
  RGB operator()(double d) const override;
  void randomize() override;
  void setRange(double min, double max) override;
  void load(const view_parameters &) override;
  void save(view_parameters &) const override;
  void create_colours();
};
} // namespace fractals