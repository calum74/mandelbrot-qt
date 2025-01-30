#include "json.hpp"
#include "nlohmann/json.hpp"
#include <sstream>

void parse(const nlohmann::json &js, fractals::view_coords::value_type &v) {
  std::stringstream ss(js.get<std::string>());
  ss >> v;
}

fractals::view_parameters read_json(const nlohmann::json &js) {
  fractals::view_parameters params;
  if (js.contains("Name"))
    params.title = js["Name"].get<std::string>();
  if (js.contains("Re"))
    parse(js["Re"], params.coords.x);
  if (js.contains("Im"))
    parse(js["Im"], params.coords.y);
  if (js.contains("Radius"))
    parse(js["Radius"], params.coords.r);
  if (js.contains("Iterations"))
    params.coords.max_iterations = js["Iterations"].get<int>();
  if (js.contains("Gradient"))
    params.colour_gradient = js["Gradient"].get<double>();
  if (js.contains("Colour"))
    params.colour_seed = js["Colour"].get<int>();
  return params;
}

nlohmann::json write_json(const fractals::view_parameters &params) {

  nlohmann::json js = {};
  js["Name"] = params.title;
  js["Colour"] = params.colour_seed;
  js["Gradient"] = params.colour_gradient;
  js["Iterations"] = params.coords.max_iterations;

  std::stringstream ss1;
  ss1 << std::setprecision(params.coords.get_precision()) << params.coords.x;
  js["Re"] = ss1.str();

  std::stringstream ss2;
  ss2 << std::setprecision(params.coords.get_precision()) << params.coords.y;
  js["Im"] = ss2.str();

  std::stringstream ss3;
  ss3 << std::setprecision(4);
  fractals::log_radius(ss3, params.coords.ln_r());
  js["Radius"] = ss3.str();

  return js;
}
