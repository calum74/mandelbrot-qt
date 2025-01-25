# Task list

Next steps:
- [ ] Mandelbrot finder based on orbits returning to the same place?
- [x] Go home to reset the colour gradient as well
- [ ] Option to reset colour gradient
- [x] Default animation type is "smooth"
- [ ] C button to cancel navigation
  - 
- [ ] Sometimes dots remain in black areas after a long calculation.
- [x] Update menu when loading a new fractal
  - ViewerWidget signal/MainUI slot for
  - fractalChanged(const char*)
  - stopAllAnimations()
- [x] Render timings should also include the setup timing.
- [ ] Crap on screen when resizing
- [ ] Have a "power" zoom as opposed to a linear zoom. 

Animation fixes:
- [ ] Bail out animation when we don't recalculate the depth
- [ ] At the very end of a zoom, don't wait for image to render, but just dump it.
- [ ] Sometimes current calculation doesn't finish if cancelAnimations(); but continues to run in the background.


Paper cuts: 
- [ ] Fix speed zoom for max iterations
  - Very fast animation can lose the max_iteration count.
  In speed zoom, we often miscalculate the final depth if we don't calculate anything
- [ ] Resize should be able to continue animation
- [ ] Errors should be stored in a separate array.
- Depth stats are bogus when dragging
- [ ] In animation, status bar is too noisy. Don't say "calculating" part.
  - Unless we are in "speed" mode...
- [ ] Status bar text is inconsistent and out of date, particularly when animating

Refactoring:
- [ ] Refactor ViewerWidget 
  - maybe migrate code to AnimatedRenderer
- [ ] Store radius in engineering format
- [ ] Hexadecimal number format

Then: JSON format
- Autosave to JSON
- Bookmarks menu
- Quick zoom to a location


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

- [ ] Surely we should colour "around" a point rather than just below/right

- [ ] Update all the other fractals to use new data types
- [ ] Too many artefacts when scaling and zooming.
  Instead, the "view" contains a pixel size.
  When the pixels in the view are larger than the new pixels, replace them.
  Still to figure out
- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.

- [ ] Multiple top-level windows

- [ ] Understand reference orbits better
  - What is the "orbit utilization"
  - What percentage of iterations
  - When is it cheaper to compute a new reference orbit?
  - Can we create a reference of a reference?
  - Can other reference orbit "split" from the old one?

- [ ] Can we "skip forward" n iterations? Is there a closed form for that??? Can we approximate the "skip forward" action using its own Taylor series??
  - What about skipping epsilon forward?

Basically it's the depth of range that causes big problems. Computing up to the minimum is usually fine.

- [ ] Colour 122 is nice

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
- [ ] Dealing with very deep zooms. E.g. scaledSmallNumber  
- [ ] Would adding another Taylor series term help?
- [ ] Can we partially evaluate the Taylor series??
- [ ] Does choice of reference orbit matter?
- [ ] Can we combine 2 reference orbits?
- [ ] Can we translate a Taylor series

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
