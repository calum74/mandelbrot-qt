# Task list

- [ ] 'q' to quit
- [ ] When initializing and resizing the viewport, ensure that we clear the extra pixels.
- Resize window -> extend or contract the view, don't zoom it.
- [ ] Current interpolation seems a bit sketchy

- Look at the state of a pixel:
  - The state encodes the error.
  - When we calculate a pixel exactly, the error = 0
  - When we scroll, we copy the error from the original pixel
  - When we assign a pixel to grey, we set the error to be MAX (e.g. 255)
  - When we zoom out, we copy the error of the original pixel
  - When we zoom in, we add one to the error of the original pixel (a bit hacky!)
  - When we interpolate, we assign the error to equal the distance from the correct value.
  - When we calculate a pixel, we check the error of the original pixel. If already zero, skip calculation!

  - Resolution (distance from a true point)
  - When calculated exactly, set it to 0 (meaning no error)
  - When scrolled, set it to the resolution of the source pixel
  - When interpolated, 

- When recalculating a pixel:
  - Look at its current resolution. If 0, skip this calculation.

- Rendering strategies
  How can we order calculations in order to use CPU cores and render partial results in good resolution?
  How can we automatically change the number of iterations?
  Use interpolation to get a smooth rendering between points.
  Multiresolution rendering.
  Rendering sequence - what's the API?
  view.render_points(Viewport&, Point * begin, Point *end);
  After rendering each point, we'll perform an RGB interpolation of the non-rendered points.
  Mark each RGB point with metadata: how accurate is this point? (to deal with threading issues)
  Threading vs rendering.
  Create a rendering job, split the view into 64 pieces.
  When we a point, we can ask about the completeness of neighbouring points

- Work item (for a thread)
  A calculated tile (x0,y0,w)
  w is 128x128 pixels.  
  Calculate using a multi-resolution approach.
  Once each resolution is calculated, interpolate the colours
  Update the view on each layer.

  Layer 0: 1x1 calculate 1 point
  Layer 1: 2x2 calculate 3 more points
  Layer 2: 4x4 = 16 - 4 = 13 more points 
  Layer 3: 8x8 = 64 - 16 = 48 more points
  Layer n: 2^nx2^n = 2^(2n) * 3/4

  How to interpolate?
  How to deal with edges?

Algorithms:
1. Split a view into 128x128 segments
2. Call the calculation with points in a specific order.

> fractal.calculate_points(Viewport &vp, const std::vector<Point> & points);

Work item:



- Calculation strategies
  Can we use relative_orbit with high_precision and delta/epsilon 

  Can we use different algorithms at different scales
  Compute early capture (maybe every 20 iterations or so)

- Perturbations, obviously



- [x] Window resizing
  - Don't crash
  - Maintain correct aspect ratio
- [ ] Mutexes around repainting?
  - Repaint whilst calculating

- Ensure we render at the highest resolution  
- Prevent zooming out too far
- Logarithmic and smooth shading
- [x] Report on render times and other stats on each frame.
- [x] Incremental rendering - when we zoom in, we'll also update the viewport already.
- [ ] Implement high precision numbers in various ways
  Including, a high-precision array.

- [ ] Efficient scrolling, rather than recalculating the entire window

1. Get the UI elements working correctly
  - [x] Drag to move
  - [x] Zoom on center
  -x[ ] Async rendering
  - [ ] Maximum zoom out
  - [x] Ensure correct aspect ratio on resize
2. Rendering
  - [x] Cancellable rendering
  - [x] Show rendering time in seconds/the framerate

Async rendering
- cancellable.
- api?

# Background

The Mandelbrot Set, is an example of a *fractal* - a mathematical pattern of unlimited complexity. It is just extremely beautiful and mesmerising.

Drawing a basic Mandelbrot set is not too complicated, but there is a lot more work required to write 