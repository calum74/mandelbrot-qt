#include "ColourMapImpl.hpp"
#include "shader.hpp"
#include "view_parameters.hpp"

fractals::ColourMapImpl::ColourMapImpl() {
  resetGradient();
  create_colours();
  update_light_source();
}

void fractals::ColourMapImpl::resetGradient() {
  params.colour_gradient = 30;
  params.colour_offset = 0;
  gradients.clear();
}

fractals::gradient_stack::result
fractals::gradient_stack::map_iteration(double d, double default_gradient,
                                        double default_offset) const {
  for (auto j = stack.rbegin(); j != stack.rend(); ++j) {
    if (d > j->iteration) {
      return {d / j->gradient + j->offset, j->gradient};
    }
  }

  return {d / default_gradient + default_offset, default_gradient};
}

fractals::RGB fractals::ColourMapImpl::operator()(double d, double dx,
                                                  double dy) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  if (!params.shading)
    return (*this)(d);

  auto colour_index =
      gradients.map_iteration(d, params.colour_gradient, params.colour_offset);

  double brightness = calculate_brightness(
      dx, dy, colour_index.gradient, params.ambient_brightness,
      params.source_brightness, light_source);

  int i = colour_index.value;
  auto f2 = colour_index.value - i;
  auto f1 = 1.0 - f2;

  i %= colours.size();
  int j = (i + 1) % colours.size();
  auto c1 = colours[i];
  auto c2 = colours[j];

  auto r = brightness * (red(c1) * f1 + red(c2) * f2);
  auto g = brightness * (green(c1) * f1 + green(c2) * f2);
  auto b = brightness * (blue(c1) * f1 + blue(c2) * f2);

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

  return make_rgb(r, g, b);
}

fractals::RGB fractals::ColourMapImpl::operator()(double d) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  auto colour_index =
      gradients.map_iteration(d, params.colour_gradient, params.colour_offset);

  int i = colour_index.value;
  auto f2 = colour_index.value - i;
  auto f1 = 1.0 - f2;

  i %= colours.size();
  int j = (i + 1) % colours.size();
  auto c1 = colours[i];
  auto c2 = colours[j];

  auto r = red(c1) * f1 + red(c2) * f2;
  auto g = green(c1) * f1 + green(c2) * f2;
  auto b = blue(c1) * f1 + blue(c2) * f2;

  return make_rgb(r, g, b);
}

void fractals::ColourMapImpl::setRange(double min, double max) {
  params.colour_gradient = (max - min) / 5.0;
  gradients.clear();
}

void fractals::ColourMapImpl::maybeUpdateRange(double min, double max) {

  if (!params.auto_gradient)
    return;

  gradients.push(max, (max - min) / 5, params.colour_gradient,
                 params.colour_offset);
}

void fractals::gradient_stack::push(double iteration, double new_gradient,
                                    double default_gradient,
                                    double default_offset) {
  // Remove any colours that are above the current max
  while (!stack.empty() && stack.back().iteration > iteration) {
    stack.pop_back();
  }

  auto last_gradient = stack.empty() ? default_gradient : stack.back().gradient;
  auto last_offset = stack.empty() ? default_offset : stack.back().offset;

  /*
    To align the colours, we need to ensure that
    iteration/last_gradient + last_offset = iteration/new_gradient + new_offset
    -> new_offset = last_offset + iteration/last_gradient -
    iteration/new_gradient
  */

  auto new_offset =
      last_offset + iteration / last_gradient - iteration / new_gradient;
  stack.push_back({iteration, new_gradient, new_offset});
}

void fractals::ColourMapImpl::randomize() {
  params.colour_scheme++;
  create_colours();
}

void fractals::ColourMapImpl::create_colours() {
  colours = generate_colours(numColours, params.colour_scheme);
}

void fractals::ColourMapImpl::load(const view_parameters &vp) {
  params.colour_scheme = vp.shader.colour_scheme;
  params.colour_gradient = vp.shader.colour_gradient;
  if (params.colour_gradient < 1.0)
    params.colour_gradient = 1.0 / params.colour_gradient;
  gradients.clear();
  create_colours();
}

void fractals::ColourMapImpl::save(view_parameters &vp) const {
  vp.shader = params;
}

void fractals::ColourMapImpl::getParameters(shader_parameters &vp) {
  vp = params;
}

std::unique_ptr<fractals::ColourMap> fractals::make_colourmap() {
  return std::make_unique<ColourMapImpl>();
}

void fractals::ColourMapImpl::setParameters(const shader_parameters &vp) {
  if (params.colour_scheme != vp.colour_scheme) {
    params.colour_scheme = vp.colour_scheme;
    create_colours();
  }

  if (params.colour_gradient != vp.colour_gradient ||
      params.colour_offset != vp.colour_offset) {
    gradients.clear();
  }

  if (params.auto_gradient && !vp.auto_gradient) {
    gradients.clear();
  }

  params = vp;
  update_light_source();
}

void fractals::ColourMapImpl::update_light_source() {
  // Recalculate the source based on spherical coordinates
  light_source = spherical_to_cartesian(params.source_direction_radians,
                                        params.source_elevation_radians);
}

fractals::unit_vector fractals::spherical_to_cartesian(double direction,
                                                       double elevation) {
  return {std::cos(elevation) * std::cos(direction),
          std::cos(elevation) *
              std::sin(direction),
          std::sin(elevation)};
}

void fractals::gradient_stack::clear()
{
  stack.clear();
}