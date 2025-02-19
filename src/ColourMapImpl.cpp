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

  d = std::pow(d, gamma) / gradient;
  int i = d;
  auto f = d - i;
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
  create_colours();
}

void fractals::ColourMapImpl::save(view_parameters &params) const {
  params.colour_seed = seed;
  params.colour_gradient = gradient;
}

std::unique_ptr<fractals::ColourMap> fractals::make_colourmap() {
  return std::make_unique<ColourMapImpl>();
}