# Task list

- [ ] Log minimum depth
- [ ] set max_iterations based on min-iterations

- [ ] Notify when complete (and the time)
- [ ] Multi-resolution algorithms - go to the next algorithm/depth when limit is reached.
  - [ ] Idea of an "evaluation strategy" which encompasses various options.
- [ ] Adaptive depth?
- [x] Higher precision using relative orbits

Base orbit is at high precision
Epsilons are calculated relatively.



- [x] 'q' to quit
- [x] When initializing and resizing the viewport, ensure that we clear the extra pixels.
- Resize window -> extend or contract the view, don't zoom it.
- [x] Current interpolation seems a bit sketchy
- [x] Correctly log the width and number of iterations
- [x] Show stats in the status bar
- [x] Update width always (not just on completion)

- [ ] Random zoom in



- Calculation strategies
  Can we use relative_orbit with high_precision and delta/epsilon 

  Can we use different algorithms at different scales
  Compute early capture (maybe every 20 iterations or so)

- Perturbations, obviously




- Ensure we render at the highest resolution  
- Prevent zooming out too far
- Logarithmic and smooth shading

- [ ] Efficient scrolling, rather than recalculating the entire window


# Background

The Mandelbrot set is a type *fractal* - a mathematical shape of unlimited complexity. The deeper you look at it, the more detail emerges. This behaves very differently to a regular shape or graph, where  when you zoom in far enough, it starts to look like a straight line.

The Mandelbrot set is defined as the set of points c that don't escape to infinity under the transformation z->z+c. In spite of the simplicity of this formula, it turns out that points arbitrarily close together can behave completely differently, a phenomenon known as the "butterfly effect". Even small variations can become amplified to the degree that given enough time, the outcome is totally different. So named, because when a butterfly beats its wings, it can eventually trigger a hurricane on the other side of the earth.

Another complexity, ehem, is that the Mandelbrot set is defined on the complex numbers. Each point in the Mandelbrot is actually on the complex plane, and is represented by x+iy. Recall that a complex number is composed of two real numbers, where one of those numbers is a multiple of i, such that i squared is -1. For the purposes of calculation however, we can ignore the fact that we are dealing with complex numbers, and just treat each point as the pair (x,y).

Drawing a basic Mandelbrot set is not too complicated, but there is a lot more work required to make this a great experience. The first problem is that the algorithm never terminates, so we need to implement a "cut-off" and stop iterating

