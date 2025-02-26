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
  colour_stack.clear();
}


fractals::RGB fractals::ColourMapImpl::operator()(double d, double dx,
                                                  double dy) const {
  if (d == 0)
    return make_rgb(0, 0, 0);

  if (!params.shading)
    return (*this)(d);

  double scaled_colour = d / params.colour_gradient + params.colour_offset;
  double scaled_gradient = params.colour_gradient;
  for (auto j = colour_stack.rbegin(); j != colour_stack.rend(); ++j) {
    if (d > j->iteration) {
      scaled_colour = d / j->gradient + j->offset;
      scaled_gradient = j->gradient;
      break;
    }
  }

  double brightness = 1.0;

  if (params.shading) {
    brightness = calculate_brightness(
        dx, dy, scaled_gradient, params.ambient_brightness,
        params.source_brightness, source_x, source_y, source_z, source_length);
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

  double scaled_colour = d / params.colour_gradient + params.colour_offset;
  double scaled_gradient = params.colour_gradient;
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
  params.colour_gradient = (max - min) / 5.0;
  colour_stack.clear();
}

void fractals::ColourMapImpl::maybeUpdateRange(double min, double max) {

  if (!params.auto_gradient)
    return;

  // Remove any colours that are above the current max
  while (!colour_stack.empty() && colour_stack.back().iteration > max) {
    colour_stack.pop_back();
  }

  // Apply the new gradient to colours above the current max,
  // so the new colours only apply to zooming in
  auto new_gradient = (max - min) / 5.0;
  auto last_gradient = colour_stack.empty() ? params.colour_gradient
                                            : colour_stack.back().gradient;
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
  colour_stack.clear();
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
    colour_stack.clear();
  }

  if (params.auto_gradient && !vp.auto_gradient) {
    colour_stack.clear();
  }

  params = vp;
  update_light_source();
}

void fractals::ColourMapImpl::update_light_source() {
  // Recalculate the source based on spherical coordinates
  source_x = std::cos(params.source_elevation_radians) *
             std::cos(params.source_direction_radians);
  source_y = std::cos(params.source_elevation_radians) *
             std::sin(params.source_direction_radians);
  source_z = std::sin(params.source_elevation_radians);
  source_length = std::sqrt(source_x * source_x + source_y * source_y +
                            source_z * source_z);
}
