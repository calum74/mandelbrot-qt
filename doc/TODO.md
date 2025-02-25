# Task list

Box:
1. Shading on/off
2. Colour scheme
3. Colour gradient

Today:
1. Fix Actions build
2. Fix CodeQL build
1. Refactor the shader
2. Shading dialog box
3. Remove shown bookmarks

```
class shader
{

};

shader make_shader(shader_parameters);
```

Bugs:
- Black regions when dragging. Can we fill it in better?
- Odd segmentation fault
1. Autonavigate stops
  Probably tried to navigate before finished rendering
2. Shows wrong background when zooming in

Control panel:

Colouring:

Animation:
Slow/medium/fast

Animation:
- Animation should reset the colour gradient

Refactoring:
- Can we refactor AsyncRenderer & AnimatedRenderer further?
- Put colourmap into mandelbrot?
  -> Only after shading done?
- Reduce need to call `convert<>` all the time.
- Colourmap and shadermap
- [ ] Split into library and fractals

Papercuts:
- Navigate randomly stops prematurely
- When we go home, reset the gradient, particularly for a zoom in
- Recolour to not recalculate the view

Colour Dialog box:
- Seed
- Gradient
- Offset

Simple shader:
- Imagine a light source at infinity
- For each pixel, look at delta-x and delta-y
- Create a normal vector of the plane
- Look at the scalar product with the plane's normal vector

Some problems:
1. Nice refactoring of pixmaps etc.
  a. Different calculation strategies
  b. Colours vs values
2. MB finding via Newton-Raphson
  - Pick a point
  - Find its closest return
  - Pick 2 adjacent points
  - Use the gradient to find the minimum??
  - When finding the brot, surely we need to find the center of 2 local orbits
4. Make it easier to add new fractals
  a. Developer mode

- [ ] Refactor Async/AnimatedRenderer
  - `animated_pixmap<T>` 

  - `abstract_animator` that can zoom anything.

  - Calculated that only outputs depths
  - Interpolator (on the depths)
  - Shader that turns depths into colours
  - Animator that performs zooms.
  - Window that displays it

- [ ] Better API for adding new fractals
- [ ] fractals -> fractools

Zooming:
- [ ] Animation bug - actually the frame buffer hasn't been transferred properly.
- Bookmarks flags can appear in the wrong place when starting a zoom in
- [ ] Bookmarks flags can appear in the wrong place when starting a zoom in
  - Thinks it's still zooming probably
- [ ] Finally fix the animation bug as well
- [ ] Animate to here still carries on zooming mysteriously
  It only happens when there is still a rendering going on?
- [ ] Zoom in goes too far
- [ ] Override zoom
  - [ ] Zoom step is always fast
  - [ ] Continuous zoom is always smooth
- [ ] Smooth zoom out
- [ ] When resizing window, copy pixels across
When we break off an animation, we lose where we were heading :-(
- Keep a record of the final zoom depth so we can just use that by default
- Zoom in goes too far
Navigate randomly now randomly stops

Scrolling:
- Bug where sometimes the gradient is set too low???
- Load bookmark -> where to set the gradient?? & reset

Bookmarks:
- [ ] Ensure they are synced between Windows
  - Loaded lazily
- Tidy up bookmarks
- Maybe sort them

Advanced mode:
- [ ] Show additional fractals
- [ ] Show number of iterations skipped


Problems:
- Use openmp
Goals for this week:
- [ ] Sort bookmarks
- [ ] Invert y axis
- [ ] Detect loops
- [ ] Adaptive colour map

Bugs:
- [ ] Multiple toplevels don't sync bookmarks
  Menus not updated properly
- [ ] Display the time without setprecision(2)??
- [ ] Bugs in New Year fractal. (imprecision of some kind)
- [x] Glitches in Magnus Opus Ex
- [ ] Suddenly a nasty judder whilst rendering.
- [ ] Scrolling disrupts depth calculations. Still not perfect
- [ ] Smooth zoom stops now if there's not enough pixels to update the depth
- [ ] Some of the bookmarks moved due to imprecision bug

Maths:
- [ ] Karatsuba multiplication
- [ ] Hexadecimal numbers
- [ ] Tidy up tests a bit
- [ ] Support zoom up to 10-e10000

Improvements:
- [ ] Use openmp
- [ ] Implement a reference Mandelbrot (perturbation only)
- [ ] Lock window
- [ ] Maybe load the bookmarks async??
- [ ] Better filename for bookmarks.json
- [ ] Eta/ progress indicator.
- [ ] Smoother zoom out??
- [ ] GPL license
- [ ] Esc to stop animations
- When we scroll, we'll sometimes lose pixels
- Sometimes shows finished calculation when it's actually calculating something new
  - Ensure that "calculating" takes priority
  - Depth 0-0 looks silly
- On open, resume previous place visited?
- [ ] "1 CPU core, "4 CPU cores", "All CPU cores"
- [ ] Rename "oversampling" to "High definition"
- [ ] Better gradient enhancements
- [ ] Shading
- [ ] Import/export bookmarks?
- [ ] Dynamic length of number?

Ideas: 
- If the Taylor series is valid, we can find a delta which makes the orbit coincide with the starting point.
- Using Newton Raphson.
- Can we approximate "32 steps" using perturbation theory?
  - Only for low epslions.
- [ ] Class OrbitConfiguration which contains all of the parameters for the orbit

# Mandelbrot explorer
Where mathematics meets art


Classification project:
a. Basic shape
  a1. mandelbrot
  a2. island
  a3. web
  a4. spiral
  a5. flowerc
  wagon-wheel
  hub
b. Order
  circular
  number of spikes
c. Description of features:
  spokes:
  wheel
b. Fractal type
  b1. regular mandelbrot
  b2. cubic mandelbrotc 

Next steps:
- [ ] Mandelbrot finder based on orbits returning to the same place?
- [ ] Option to reset colour gradient

- [ ] Make naming consistent with camelcase.
- [ ] rename autozoom to autonavigate

Paper cuts: 
- [ ] Fix speed zoom for max iterations
  - Very fast animation can lose the max_iteration count.
- [ ] Resize should be able to continue animation
- [ ] Errors should be stored in a separate array.
- Depth stats are bogus when dragging
- [ ] In animation, status bar is too noisy. Don't say "calculating" part.
  - Unless we are in "speed" mode...
- [ ] Status bar text is inconsistent and out of date, particularly when animating

Refactoring:
- [ ] Store radius in engineering format
- [ ] Hexadecimal number format


Then: Windows installed MSIX package

- [ ] Zoom out smoothly as well?

3. Implement bookmarks using json
  Zoom to a bookmark

Autocolourmap:
- 2 modes: auto and manual

# Other

- [ ] Rendering glitches on power 3 mandelbrot

- [ ] Get a deeper series by looking more closely at the divergence criteria
- [ ] Get a deeper orbit by adding more terms?

Bookmarks.json file.
- Store the radius in engineering form.

Features:

- [ ] Better center and mandelbrot finder
- [ ] Auto-enhance colours
  How to make this a smooth/auto experience?

- [ ] fractals:: namespace is stupid



- [ ] Split up orbit.hpp

- [ ] Make it nicer to adapt Mandelbrot to Mandeldrop

- [x] Surely we should colour "around" a point rather than just below/right

- [ ] Update all the other fractals to use new data types
- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.


- [ ] Understand reference orbits better
  - What is the "orbit utilization"
  - What percentage of iterations are skipped
  - When is it cheaper to compute a new reference orbit?
  - Can we create a reference of a reference?
  - Can other reference orbit "split" from the old one?

- [ ] Can we "skip forward" n iterations? Is there a closed form for that??? Can we approximate the "skip forward" action using its own Taylor series??
  - What about skipping epsilon forward?

Basically it's the depth of range that causes big problems. Computing up to the minimum is usually fine.

- [ ] Refactor mandelbrot parameters, for example use real_number.
Maybe there's a hard-coded way to express the delta range? So we can scale all deltas by M, e.g. bias them all by 500.

Optimization:
- [ ] Be less strict about rejecting invalid iteration counts
- [ ] Look into deeper zooms
- [ ] Visualize skipped iterations. Could be a clue into where to place the reference orbit

## Implement a bookmarks feature.
Have a saved library of interesting fractals to look at.

- [ ] Auto-enhance image if it's too dim - the range is unsuitable

- [ ] Load file to change the menu to show current bookmark

Short term goals:
- [ ] MSIX installer

Ideas:
- [ ] Instant recolour
- [ ] Performance improvements

Blockers:
- [ ] Windows installer
- [ ] Building Qt from source on Windows still does not work

Bugs:
- [ ] Still some zoom inaccuracy on Mandeldrop around 1e-20

Colouring:
- [x] Find a new default seed

Enhancements:
- [ ] Display the gradient somewhere
- [ ] Create benchmarks
- [ ] Make sure to not recalculate calculated pixels (e.g. scroll)
- [ ] Progress bar somewhere (but why?)

Documentation and tidy:
- [ ] Further code tidy
- [ ] More unit tests, for example, orbits
  - Mandelbrot - list the classes and document them

# Performance ideas

- [ ] Read up on state of the art again
- [ ] Look at where the performance is currently going
- [ ] Dealing with very large number of iterations
- [x] Dealing with very deep zooms. E.g. scaledSmallNumber  
- [x] Would adding another Taylor series term help?
- [ ] Can we partially evaluate the Taylor series??
- [ ] Does choice of reference orbit matter?
- [ ] Can we combine 2 reference orbits?
- [ ] Can we translate a Taylor series efficiently?

# Long term tasks

- Tidy up the installer text and license
- [ ] Progress bar

High precision tidy:
- Express size in bits
- Configure integer size as well

Less important tasks:
- [ ] Errors in pixels affect the output, so store them in a separate array

Bugs:
- When opening 'Go to' window, ensure focus is on the ok button

Testing:
- [ ] Orbits tests
- [ ] High precision tests

Game: Count mini-brots

Refactoring:
- [ ] Ability to iterate all points simultaneously so that the black can shrink.
- [ ] Unit-test orbits - see how far they diverge from a reference orbit.
- [ ] Still some glitches with epsilon

Version 2.0:
- [ ] Center finding

Version 3.0:
- [ ] Load/save position
  - Save palette
  - Save algorithm
- [ ] Export to image
- [ ] Generate movie
- [ ] Edit colour palette
