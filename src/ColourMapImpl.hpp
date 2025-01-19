#include "ColourMap.hpp"
#include "fractal.hpp"
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  std::vector<RGB> colours;
  double gradient = 0.08, gamma = 0.8;
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