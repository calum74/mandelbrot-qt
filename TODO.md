# Task list

MVP tasks:
- [ ] Build tasks
   - open-source
   - CPack maybe
- [ ] Set an icon

Coding tasks:
- Refactor view layout logic
- General code tidy

Bugs:
- Around 1e-30, the view goto logic is buggy
- You should be able to go-to the same location without moving anywhere
- When increasing iterations should reset the status bar


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

Observation:
- Around minibrots, you have local rotation symmetry of order 2


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


