#include "ColourMap.hpp"
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  std::vector<RGB> colours;
  double gradient = 30, gamma = 1.0;
  unsigned int seed = 168;
  unsigned int numColours = 100;

  ColourMapImpl();
  RGB operator()(double d) const override;
  void randomize() override;
  void resetGradient() override;
  void setRange(double min, double max) override;
  void load(const view_parameters &) override;
  void save(view_parameters &) const override;
  void create_colours();
};
} // namespace fractals