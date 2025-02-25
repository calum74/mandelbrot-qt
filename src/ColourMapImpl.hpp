#include "ColourMap.hpp"
#include <vector>

namespace fractals {

class ColourMapImpl : public ColourMap {
public:
  ColourMapImpl();
  RGB operator()(double d, double dx, double dy) const override;
  RGB operator()(double d) const override;
  void randomize() override;
  void resetGradient() override;
  void setRange(double min, double max) override;
  void maybeUpdateRange(double min, double max) override;
  void load(const view_parameters &) override;
  void save(view_parameters &) const override;

  void setParameters(const shader_parameters &) override;
  void getParameters(shader_parameters &params) override;

private:
  void create_colours();

  std::vector<RGB> colours;
  double gradient = 30;
  double offset = 0;
  unsigned int seed = 168;
  unsigned int numColours = 100;
  bool auto_gradient = true;
  bool shading = true;

  // The `gradient` and `offset` are used for all colours above `iteration`
  struct colour_entry {
    double iteration, gradient, offset;
  };

  std::vector<colour_entry> colour_stack;
};
} // namespace fractals