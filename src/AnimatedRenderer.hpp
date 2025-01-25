#pragma once
#include "ColourMap.hpp"
#include "RGB.hpp"
#include "Renderer.hpp"
#include "registry.hpp"

#include <atomic>
#include <chrono>

class AnimatedRenderer {
public:
  AnimatedRenderer();
  ~AnimatedRenderer();

  void calculate_async(fractals::Viewport &output);

  std::unique_ptr<fractals::Registry> registry;

  // Note destruction order - renderer must be destroyed after viewport
  std::unique_ptr<fractals::Renderer> renderer;

  bool zooming = false;
  std::atomic<bool> calculationFinished = false;
  std::atomic<bool> zoomTimeout = false;
  std::chrono::time_point<std::chrono::system_clock> zoom_start;
  std::chrono::duration<double> zoom_duration;
  int zoom_x, zoom_y;
  double zoomtopoint_limit;

  enum class AnimationType {
    none,
    autozoom,
    zoomtopoint,
    zoomatcursor
  } current_animation = AnimationType::none;

  double estimatedSecondsPerPixel = 0;
  std::vector<fractals::RGB> previousImagePixels, backgroundImagePixels;

  std::unique_ptr<fractals::ColourMap> colourMap;

  bool fixZoomSpeed = false;
  std::chrono::duration<double> fixZoomDuration;
};