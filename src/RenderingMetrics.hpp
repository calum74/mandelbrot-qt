#pragma once

namespace fractals {
// For reporting on what happened during rendering a view
struct RenderingMetrics {
  // These metrics are always reported
  double log_radius; // The natural log of the radius
  double min_depth;
  double max_depth;
  std::size_t points_calculated;
  std::uint64_t non_black_points;
  double seconds_per_point;

  double discovered_depth; // The depth of the 99th percentile pixel

  bool fully_evaluated; // True if the calculation evaluated every point

  // These metrics are reported only on successful completion
  double avg_iterations;
  double avg_skipped_iterations;
  double render_time_seconds;
};
} // namespace fractals