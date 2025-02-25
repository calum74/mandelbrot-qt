#include "ColourMap.hpp"
#include "shader_parameters.hpp"
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

  const unsigned int numColours = 100;
  std::vector<RGB> colours;
  shader_parameters params;

  // The `gradient` and `offset` are used for all colours above `iteration`
  struct colour_entry {
    double iteration, gradient, offset;
  };

  std::vector<colour_entry> colour_stack;
};
} // namespace fractals