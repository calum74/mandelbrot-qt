#include "nlohmann/json_fwd.hpp"
#include "view_parameters.hpp"

fractals::view_parameters read_json(const nlohmann::json &js);
nlohmann::json write_json(const fractals::view_parameters &params);
