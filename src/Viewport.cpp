#include "Viewport.hpp"

fractals::Viewport::iterator fractals::Viewport::begin() { return data; }

fractals::Viewport::iterator fractals::Viewport::end() { return data + size(); }

int fractals::Viewport::size() const { return width * height; }

void fractals::Viewport::calculation_started(double log_radius,
                                             int iterations) {}

void fractals::Viewport::schedule_next_calculation() {}

void fractals::Viewport::start_timer() {}