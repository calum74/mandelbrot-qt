# Task list

Next up:
- [ ] Threading
  - UI freeze sometimes
    Problem with the mouse??
  - Rendering glitches

- [ ] Auto zoom in based on cursor
- [ ] Multiresolution magic
- [ ] CPack
- [ ] Open-source mandelbrot algorithms?

Core work:
- [x] Use binary search to locate the starting point better (1)
- [ ] Log the starting iteration in the task bar (2)
- [ ] Build on Actions if possible & upload artefacts (3)
- [ ] Create a `class Algorithm` which is different to `Fractal` (4)
- [x] Refactor orbit.hpp (5)
- [ ] Figure out epsilon
- [ ] Multi-resolution rendering/switch algorithm at different resolutions
- [ ] Multithreading
- [ ] Fix precision issues/glitches. Maybe check that the low epsilon was just a fluke and we need to validate all values.

Algorithm improvements:
- [ ] Fix up the colour palette.
- [ ] Smooth shading
- [ ] Shadows
- [ ] Auto-depth

UI improvements:
- [ ] Ability to add new fractals (1,2,3...)
- [ ] Reset back to start
- [ ] Tweak iterations
- [ ] Tweak options, e.g. speed or precision
- [ ] Tweak colour palette
- [ ] Shift colour palette
- [ ] Random explore
- [ ] Ensure we render image at the highest resolution  
- [ ] Prevent zooming out too far


Random:
- [ ] set max_iterations based on min-iterations
- [ ] Multi-resolution algorithms - go to the next algorithm/depth when limit is reached.
  - [ ] Idea of an "evaluation strategy" which encompasses various options.

Version 2.0:
- [ ] Load/save position
- [ ] Export to image
- [ ] Generate movie
- [ ] Smooth shading


# Notes

Rendering & threading:
- Problem with async updates is that drawing directly to the view is a bad idea.
- Solution is for each thread to render every nth pixel
- Each thread gets assigned every nth 16x16 region and only renders those
- However this could race between threads, so each thread owns a 16x16 pixel

- Only update the display once each stage is completed


  - How to detect?
  - When a thread finishes a stage, it subtracts the stage from the total.
  - On round number

- How to sync threads

Algorithm?
1) 