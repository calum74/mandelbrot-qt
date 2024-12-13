# Task list

- [ ] Fix the crash at high zoom
- [ ] Auto zoom in based on cursor
- [ ] Multiresolution magic
- [ ] CPack
- [ ] Open-source mandelbrot repo

Core work:
- [x] Use binary search to locate the starting point better (1)
- [ ] Log the starting iteration in the task bar (2)
- [ ] Build on Actions if possible & upload artefacts (3)
- [ ] Create a `class Algorithm` which is different to `Fractal` (4)
- [x] Refactor orbit.hpp (5)
- [x] Figure out epsilon
- [ ] Multi-resolution rendering/switch algorithm at different resolutions
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
- [ ] Prevent zooming out too far
- [ ] Initial position should be in the center
- [ ] Window resizing correctly

Random:
- [ ] set max_iterations based on min-iterations
- [ ] Multi-resolution algorithms - go to the next algorithm/depth when limit is reached.
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

Multiresolution design
When zooming in or out, a fractal can report on whether it has reached its limit.
? How to hand over the coordinates from one fractal to another?

E.g.


A fractal can report on its 