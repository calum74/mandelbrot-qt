#include "ColourMapImpl.hpp"
#include "shader.hpp"
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

fractals::RGB fractals::ColourMapImpl::operator()(double d, double dx,
                                                  double dy) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  if(!shading)
    return (*this)(d);

  double scaled_colour = d / gradient;
  double scaled_gradient = gradient;
  for (auto j = colour_stack.rbegin(); j != colour_stack.rend(); ++j) {
    if (d > j->iteration) {
      scaled_colour = d / j->gradient + j->offset;
      scaled_gradient = j->gradient;
      break;
    }
  }

  double brightness = 1.0;

  if (shading) {
    // TODO: Optimize this
    dx /= scaled_gradient;
    dy /= scaled_gradient;

    // A unit shade vector
    double shade_x = 1, shade_y = 1, shade_z = 1;
    double shade_length =
        std::sqrt(shade_x * shade_x + shade_y * shade_y + shade_z * shade_z);

    /*
      v1 = (dx, 0, 1
      v2 = (0, dy, 1)
      v3 = v1 X v2 = (dy, -dx, dx*dy)
      v3 = (v3.x, v3.y, v3.z) / |v3|
      brightness = ambient + (1-ambient)(v3 . shade)
    */

    double surface_normal_x = dy;
    double surface_normal_y = -dx;
    double surface_normal_z = dx * dy;
    double surface_normal_length =
        std::sqrt(surface_normal_x * surface_normal_x +
                  surface_normal_y * surface_normal_y +
                  surface_normal_z * surface_normal_z);
    if (surface_normal_length == 0) {
      surface_normal_length = 1;
      // surface_normal_z = 1;
    }

    double dot_product =
        (surface_normal_x * shade_x + surface_normal_y * shade_y +
         surface_normal_z * shade_z) /
        (surface_normal_length * shade_length);
    double ambient_brightness = 0.4;
    double saturation_factor = 0.7; // 0.5;
    brightness = ambient_brightness + (1.0 - ambient_brightness) *
                                          (1.0 + dot_product) *
                                          saturation_factor;
    // if (brightness > 1)
    //   brightness = 1;
    // if (brightness < 0)
    //   brightness = 0;
  }

  int i = scaled_colour;
  auto f2 = scaled_colour - i;
  auto f1 = 1.0 - f2;

  i %= colours.size();
  int j = (i + 1) % colours.size();
  auto c1 = colours[i];
  auto c2 = colours[j];

  auto r = brightness * (red(c1) * f1 + red(c2) * f2);
  auto g = brightness * (green(c1) * f1 + green(c2) * f2);
  auto b = brightness * (blue(c1) * f1 + blue(c2) * f2);
// Allow saturation
#if 1
  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;

  if (r < 0)
    r = 0;
  if (g < 0)
    g = 0;
  if (b < 0)
    b = 0;
#endif

  return make_rgb(r, g, b);
}

fractals::RGB fractals::ColourMapImpl::operator()(double d) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  double scaled_colour = d / gradient;
  double scaled_gradient = gradient;
  for (auto j = colour_stack.rbegin(); j != colour_stack.rend(); ++j) {
    if (d > j->iteration) {
      scaled_colour = d / j->gradient + j->offset;
      scaled_gradient = j->gradient;
      break;
    }
  }

  double brightness = 1.0;

  int i = scaled_colour;
  auto f2 = scaled_colour - i;
  auto f1 = 1.0 - f2;

  i %= colours.size();
  int j = (i + 1) % colours.size();
  auto c1 = colours[i];
  auto c2 = colours[j];

  auto r = brightness * (red(c1) * f1 + red(c2) * f2);
  auto g = brightness * (green(c1) * f1 + green(c2) * f2);
  auto b = brightness * (blue(c1) * f1 + blue(c2) * f2);

  return make_rgb(r, g, b);
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

void fractals::ColourMapImpl::enableAutoGradient() { auto_gradient = true; }

void fractals::ColourMapImpl::disableAutoGradient() {
  auto_gradient = false;
  colour_stack.clear();
}

void fractals::ColourMapImpl::enableShading() { shading = true; }

void fractals::ColourMapImpl::disableShading() { shading = false; }
