#include "ColourMapImpl.hpp"

fractals::ColourMapImpl::ColourMapImpl() { randomize(); }

fractals::RGB fractals::ColourMapImpl::operator()(double d) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  d = offset + std::pow(d, gamma) * k;
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

void fractals::ColourMapImpl::setRange(double, double) {}

void fractals::ColourMapImpl::randomize() {
  std::uniform_int_distribution<int> r256(0, 256);

  // Create 20 random colours
  std::vector<RGB> newColours(20);
  for (auto &c : newColours) {
    c = make_rgb(r256(e), r256(e), r256(e));
  }

  colours = std::move(newColours);
}

std::unique_ptr<fractals::ColourMap> fractals::make_colourmap() {
  return std::make_unique<ColourMapImpl>();
}