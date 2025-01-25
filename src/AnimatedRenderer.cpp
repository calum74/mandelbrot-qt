#include "AnimatedRenderer.hpp"
#include "Renderer.hpp"
#include "registry.hpp"

void register_fractals(fractals::Registry &r);

AnimatedRenderer::AnimatedRenderer()
    : colourMap{fractals::make_colourmap()},
      registry{fractals::make_registry()},
      renderer{fractals::make_renderer(*registry)} {

  register_fractals(*registry);
}

AnimatedRenderer::~AnimatedRenderer() { renderer.reset(); }

void AnimatedRenderer::calculate_async(fractals::Viewport &vp) {
  renderer->calculate_async(vp, *colourMap);
}
