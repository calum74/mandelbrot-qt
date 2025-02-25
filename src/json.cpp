#include "json.hpp"
#include "nlohmann/json.hpp"
#include <sstream>

fractals::view_parameters read_json(const nlohmann::json &js) {
  fractals::view_parameters params;
  if (js.contains("Name"))
    params.title = js["Name"].get<std::string>();
  if (js.contains("Re"))
    params.x = js["Re"].get<std::string>();
  if (js.contains("Im"))
    params.y = js["Im"].get<std::string>();
  if (js.contains("Radius"))
    params.r = js["Radius"].get<std::string>();
  if (js.contains("Iterations"))
    params.max_iterations = js["Iterations"].get<int>();
  if (js.contains("Gradient"))
    params.shader.colour_gradient = js["Gradient"].get<double>();
  if (js.contains("Colour"))
    params.shader.colour_scheme = js["Colour"].get<int>();
  if (js.contains("Algorithm"))
    params.algorithm = js["Algorithm"].get<std::string>();
  return params;
}

nlohmann::json write_json(const fractals::view_parameters &params) {
  nlohmann::json js = {};
  js["Name"] = params.title;
  js["Colour"] = params.shader.colour_scheme;
  js["Gradient"] = params.shader.colour_gradient;
  js["Iterations"] = params.max_iterations;
  js["Algorithm"] = params.algorithm;
  js["Re"] = params.x;
  js["Im"] = params.y;
  js["Radius"] = params.r;
  return js;
}
