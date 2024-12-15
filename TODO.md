# Task list

MVP tasks:
- [x] Menu bar
- [ ] Open-source mandelbrot repo
- [ ] CPack
- [ ] Copy/Paste coords

- Around 1e-30, the view goto logic is buggy
- You should be able to go-to the same location without moving anywhere


View management logic should be abstract

When we zoom out, we should also reduce the iteration count

Testing:
- [ ] Orbits tests
- [ ] High precision tests


Refactoring:
- [ ] Abstract the idea of a view/algorithm and reference orbit

- [ ] Smooth rendering.
- [ ] Unit-test orbits - see how far they diverge from a reference orbit.

- Zoom out to use a slower algorithm?
- [ ] Still some glitches with epsilon
- Load a coord (in decimal form)

Core work:
- [x] Use binary search to locate the starting point better (1)
- [ ] Log the starting iteration in the task bar (2)
- [ ] Build on Actions if possible & upload artefacts (3)
- [ ] Create a `class Algorithm` which is different to `Fractal` (4)
- [x] Refactor orbit.hpp (5)
- [x] Figure out epsilon
- [x] Multi-resolution rendering/switch algorithm at different resolutions
- [x] Multithreading
- [x] Fix precision issues/glitches. Maybe check that the low epsilon was just a fluke and we need to validate all values.

Algorithm improvements:
- [x] Fix up the colour palette.
- [ ] Smooth shading
- [ ] Shadows
- [ ] Auto-depth
- [ ] Turn rendering_sequence into an iterator

UI improvements:
- [ ] Ability to add new fractals (1,2,3...)
- [ ] Reset back to start
- [ ] Tweak iterations
- [ ] Tweak options, e.g. speed or precision
- [ ] Random explore
- [ ] Ensure we render image at the highest resolution  
- [x] Prevent zooming out too far
- [x] Initial position should be in the center
- [x] Window resizing correctly
- [ ] Get an app icon
- [ ] Create an installer package

Random:
- [x] set max_iterations based on min-iterations
- [x] Multi-resolution algorithms - go to the next algorithm/depth when limit is reached.
  - [ ] Idea of an "evaluation strategy" which encompasses various options.

Version 2.0:
- [ ] Load/save position
- [ ] Export to image
- [ ] Generate movie
- [ ] Smooth shading
- [ ] Auto-iterations
- [ ] Smooth navigation
- [ ] Tweak colour palette
- [ ] Shift colour palette


# Notes


## Smooth zooming



In quality mode, we only allow zooming when we have fi



The current viewport scaling idea is quick and ugly. It's made worse by threading.

When we zoom, we need to smoothly project pixels onto the plane.

We need to create a sense of total immersion.

