#include "ColourMapImpl.hpp"
#include "view_parameters.hpp"
#include <random>

// Good ones: 31, 83, 97, 106, 112
// We initialze the random number generator with a seed that produces
// attractive colours.
fractals::ColourMapImpl::ColourMapImpl() {
  seed--;
  resetGradient();
  randomize();
}

void fractals::ColourMapImpl::resetGradient() { gradient = 30; }

fractals::RGB fractals::ColourMapImpl::operator()(double d) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  double scaled_colour = d / gradient;
  for (auto j = colour_stack.rbegin(); j != colour_stack.rend(); ++j) {
    if (d > j->iteration) {
      scaled_colour = d / j->gradient + j->offset;
      break;
    }
  }
  int i = scaled_colour;
  auto f = scaled_colour - i;
  i %= colours.size();
  int j = (i + 1) % colours.size();
  auto c1 = colours[i];
  auto c2 = colours[j];

  return make_rgb(red(c1) * (1 - f) + red(c2) * f,
                  green(c1) * (1 - f) + green(c2) * f,
                  blue(c1) * (1 - f) + blue(c2) * f);
}

void fractals::ColourMapImpl::setRange(double min, double max) {
  gradient = (max - min) / 5.0;
  colour_stack.clear();
}

void fractals::ColourMapImpl::maybeUpdateRange(double min, double max) {

  if (!auto_gradient)
    return;

  // Remove any colours that are above the current max
  while (!colour_stack.empty() && colour_stack.back().iteration > max) {
    colour_stack.pop_back();
  }

  // Apply the new gradient to colours above the current max,
  // so the new colours only apply to zooming in
  auto new_gradient = (max - min) / 5.0;
  auto last_gradient =
      colour_stack.empty() ? gradient : colour_stack.back().gradient;
  auto last_offset = colour_stack.empty() ? 0 : colour_stack.back().offset;

  /*
    To align the colours, we need to ensure that
    max/last_gradient + last_offset = max/new_gradient + new_offset
    -> new_offset = last_offset + max/last_gradient - max/new_gradient
  */

  auto new_offset = last_offset + max / last_gradient - max / new_gradient;
  colour_stack.push_back({max, new_gradient, new_offset});
}

void fractals::ColourMapImpl::randomize() {
  seed++;
  create_colours();
}

void fractals::ColourMapImpl::create_colours() {
  // If we want to find a new seed, log it here
  // std::cout << "Colour is " << seed << std::endl;
  std::mt19937 e{seed};

  // Create 20 random colours
  std::vector<RGB> newColours(numColours);
  for (auto &c : newColours) {
    c = e() & 0xffffff;
  }

  colours = std::move(newColours);
}

void fractals::ColourMapImpl::load(const view_parameters &params) {
  seed = params.colour_seed;
  gradient = params.colour_gradient;
  if (gradient < 1.0)
    gradient = 1.0 / gradient;
  colour_stack.clear();
  create_colours();
}

void fractals::ColourMapImpl::save(view_parameters &params) const {
  params.colour_seed = seed;
  params.colour_gradient = gradient;
}

std::unique_ptr<fractals::ColourMap> fractals::make_colourmap() {
  return std::make_unique<ColourMapImpl>();
}

void fractals::ColourMapImpl::enableAutoGradient() {
  auto_gradient = true;
}

void fractals::ColourMapImpl::disableAutoGradient() {
  auto_gradient = false;
  colour_stack.clear();
}
