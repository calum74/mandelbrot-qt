

# Task list

- [ ] Window resizing
  - Don't crash
  - Maintain correct aspect ratio
- Ensure we render at the highest resolution  
- Prevent zooming out too far
- Logarithmic and smooth shading
- [x] Report on render times and other stats on each frame.
- [ ] Incremental rendering - when we zoom in, we'll also update the viewport already.
- [ ] Implement high precision numbers in various ways
  Including, a high-precision array.

- [ ] Efficient scrolling, rather than recalculating the entire window

1. Get the UI elements working correctly
  - [x] Drag to move
  - [x] Zoom on center
  - [ ] Async rendering
  - [ ] Maximum zoom out
  - [ ] Ensure correct aspect ratio on resize
2. Rendering
  - [x] Cancellable rendering
  - [ ] Show rendering time in seconds/the framerate

Async rendering
- cancellable.
- api?