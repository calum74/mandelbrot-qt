#pragma once
#include <cstddef>

namespace fractals {
// For reporting on what happened during rendering a view
struct RenderingMetrics {
  bool last_action_was_a_scroll;

  // These metrics are always reported
  double log_radius; // The natural log of the radius
  double min_depth;
  double max_depth;
  std::size_t points_calculated;
  std::size_t non_black_points;
  double seconds_per_point;

  double discovered_depth; // The depth of the 99th percentile pixel

  bool fully_evaluated; // True if the calculation evaluated every point

  // These metrics are reported only on successful completion
  double average_iterations;
  double average_skipped_iterations;
  double render_time_seconds;
};
} // namespace fractals