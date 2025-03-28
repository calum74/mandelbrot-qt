# Task list

Next steps:

struct point { int x, y, iteration };

std::vector<point> get_orbit(point);


Glitches:
- [ ] Still glitching whilst rendering
  Maybe the error is wrong when interpolation fails

- [ ] Random navigation gets lost
- Flickering whilst animating
  - Due to read/write barriers??
- Report JSON errors better (not just a seg fault)

# Release 1 checklist

1. Black squares when navigating
3. Navigate gets lost
4. Sync bookmarks correctly
5. Curate bookmarks

# Future ideas

Show orbits
-> Gives us a center of rotation??
-> Gives us an order of rotation??


Context menu?


? What happens if we differentiate the MB set function?

dx/di = 2z 

Could we have a programming language where all variables are types?

type duration: int
E.g. fn wait = duration -> sleep(duration)

  duration = 123;  // A local variable
  my_duration = 123  //

Idea: Most images have rotational symmetry, so if we can figure out the rotational symmetry, then we can render multiple points at the same time.

As we zoom, the shadows change. This seems very unrealistic. How can we change this??

Implement "discovered depth" again.

- Implement a "simple view" which just performs the calculations

- Display stars instead of blackness for the minibrots?

Papercuts:
- Navigate randomly stops prematurely
- When we go home, reset the colour gradient, particularly for a zoom in

Some problems:
2. MB finding via Newton-Raphson
  - Pick a point
  - Find its closest return
  - Pick 2 adjacent points
  - Use the gradient to find the minimum??
  - When finding the brot, surely we need to find the center of 2 local orbits
4. Make it easier to add new fractals
  a. Developer mode

- [ ] Better API for adding new fractals
- [ ] fractals -> fractools or anything else

Zooming:
- [ ] When resizing window, copy pixels across
When we break off an animation, we lose where we were heading :-(
- Keep a record of the final zoom depth so we can just use that by default

Scrolling:
- Bug where sometimes the gradient is set too low???

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
- [ ] Invert y axis
- [ ] Detect loops

Bugs:
- [ ] Multiple toplevels don't sync bookmarks
  Menus not updated properly
- [ ] Bugs in New Year fractal. (imprecision of some kind)
- [ ] Suddenly a nasty judder whilst rendering.
- [ ] Scrolling disrupts depth calculations. Still not perfect
- [ ] Smooth zoom stops now if there's not enough pixels to update the depth

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
- [ ] Import/export bookmarks?
- [ ] Dynamic length of number?

Ideas: 
- If the Taylor series is valid, we can find a delta which makes the orbit coincide with the starting point.
- Using Newton Raphson.
- Can we approximate "32 steps" using perturbation theory?
  - Only for low epslions.
- [ ] Class OrbitConfiguration which contains all of the parameters for the orbit


# Classification project:
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
# Other

- [ ] Rendering glitches on power 3 mandelbrot

- [ ] Get a deeper series by looking more closely at the divergence criteria
- [ ] Get a deeper orbit by adding more terms?


Features:

- [ ] Better center and mandelbrot finder

- [ ] fractals:: namespace is stupid

- [ ] Split up orbit.hpp

- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.
  A bit slow!

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

Short term goals:
- [ ] MSIX installer

Blockers:
- [ ] Windows installer
- [ ] Building Qt from source on Windows still does not work

Enhancements:
- [ ] Display the gradient somewhere
- [ ] Create benchmarks
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

Bugs:
- When opening 'Go to' window, ensure focus is on the ok button

Testing:
- [ ] Orbits tests
- [ ] High precision tests

Refactoring:
- [ ] Ability to iterate all points simultaneously so that the black can shrink.
- [ ] Unit-test orbits - see how far they diverge from a reference orbit.
- [ ] Still some glitches with epsilon

Version 2.0:
- [ ] Center finding

Version 3.0:
- [ ] Generate movie
- [ ] Edit colour palette
