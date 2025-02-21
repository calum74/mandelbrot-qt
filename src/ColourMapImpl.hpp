#include "ColourMap.hpp"
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  ColourMapImpl();
  RGB operator()(double d) const override;
  void randomize() override;
  void resetGradient() override;
  void setRange(double min, double max) override;
  void maybeUpdateRange(double min, double max) override;
  void load(const view_parameters &) override;
  void save(view_parameters &) const override;
  void create_colours();

private:
  std::vector<RGB> colours;
  double gradient = 30;
  unsigned int seed = 168;
  unsigned int numColours = 100;

  // The `gradient` and `offset` are used for all colours above `iteration`
  struct colour_entry {
    double iteration, gradient, offset;
  };

  std::vector<colour_entry> colour_stack;
};
} // namespace fractals