#include "ColourMap.hpp"
#include "shader.hpp"
#include "shader_parameters.hpp"
#include <vector>

namespace fractals {

class gradient_stack {
public:
  struct result {
    double value;
    double gradient;
  };

  result map_iteration(double iteration, double default_gradient, double default_offset) const;
  void push(double iteration, double new_gradient, double default_gradient, double default_offset);
  void clear();

private:
  struct entry {
    double iteration;
    double gradient;
    double offset;
  };
  entry default_entry;
  std::vector<entry> stack;
};


RGB get_colour(const std::vector<RGB> &colours, double iteration,
               double gradient, double offset, double brightness);

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
  void update_light_source();

  shader_parameters params;
  const unsigned int numColours = 100;
  std::vector<RGB> colours;
  gradient_stack gradients;
  unit_vector light_source;
};
} // namespace fractals
