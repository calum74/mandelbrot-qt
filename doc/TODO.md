# Task list

- [ ] Fractals should name themselves?
- [ ] `namespace fractools`
- [ ] How to register a new fractal?

```c++

class myfrac {};

const fractals::pointwise_fractal &mandelbrot7_fractal = fractools::make_fractal<>();

void my_fractals()
{
  register_fractal(mandelbrot7_fractal);
  whatever here...
}
```

make_fractal could do something? e.g. create a static.




Better API for adding new fractals!

Refactor:
- [ ] New namespaces
  - `fractals` is so lame
  - 'fractal.hpp` is also lame


When we break off an animation, we lose where we were heading :-(
- Keep a record of the final zoom depth so we can just use that by default
- Resize window - super tricky!

- [ ] Tidy up and document header files
- [ ] Documentation


Problems:

- Bookmarks flags can appear in the wrong place when starting a zoom in

- Tidy up bookmarks
- Maybe sort them
- Plant flags on all the bookmarks.
- Keep breadcrumbs of where we have been?
  - Size of icon relates to distance
    Return the difference of the logs of the radius.
    If negative, hide it.
    Size = k * log(size_ratio)
  - Don't show bookmark if we've gone past it
- Use openmp
Goals for this week:
- [ ] Finally fix the animation bug as well
- [ ] Bookmarks flags can appear in the wrong place when starting a zoom in
  - Thinks it's still zooming probably

- [ ] Sort bookmarks
- [ ] Invert y axis
- [ ] Detect loops
- [ ] Adaptive colour map


Bugs:
- [ ] Multiple toplevels don't sync bookmarks
  Menus not updated properly
- [ ] Zoom in goes too far
- [ ] Display the time without setprecision(2)??
- [ ] Bugs in New Year fractal. (imprecision of some kind)
- [x] Glitches in Magnus Opus Ex
- [ ] Animate to here still carries on zooming mysteriously
  It only happens when there is still a rendering going on?
- [ ] Suddenly a nasty judder whilst rendering.
- [ ] Scrolling disrupts depth calculations. Still not perfect
- [ ] Smooth zoom stops now if there's not enough pixels to update the depth
- [ ] Some of the bookmarks moved due to imprecision bug

Improvements:
- [ ] Use openmp
- [ ] Implement a reference Mandelbrot (perturbation only)
- [ ] Lock window
- [ ] Karatsuba multiplication
- [ ] Carry on animating during window resizing
- [ ] Maybe load the bookmarks async??
- [ ] Comparative MB sets - can we look at 2 regions side-by-side?
  - New toplevel window
- [ ] Better filename for bookmarks.json
- [ ] Zoom step is always fast?
- [ ] Continuous zoom is always smooth
- [ ] Eta/ progress indicator.
- [ ] Smoother zoom out??
- [ ] GPL license
- [ ] Esc to stop animations
- When we scroll, we'll sometimes lose pixels
- Sometimes shows finished calculation when it's actually calculating something new
  - Ensure that "calculating" takes priority
  - Depth 0-0 looks silly
- Better normalization logic for high_exponent_real
- On open, resume previous place visited?
- [ ] "1 CPU core, "4 CPU cores", "All CPU cores"
- [ ] Rename "oversampling" to "High definition"
- [ ] Better gradient enhancements
- [ ] Shading
- [ ] Import/export bookmarks?
- [ ] Show number of iterations skipped
- [ ] Put a "flag" icon next to each bookmark
- [ ] Dynamic length of number?
- [ ] Smooth zoom out
- [ ] Support zoom up to 10-e10000

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
- [ ] Sometimes dots remain in black areas after a long calculation.

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
