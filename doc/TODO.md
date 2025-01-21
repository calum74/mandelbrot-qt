# Task list

Plan for today:
1. Refactor AsyncRender into header files
2. Create an AnimatedRanderer with additional timing capabilities
3. Smooth zoom to final point
Refactor:
- Fractals create a calculation Factory which is non-const.
- RenderingSequence

```c++
class rendering_sequence
{
  rendering_sequence(int width, int height, int start_size);

  std::pair<int, int> layer(int layer_size);

  struct iterator
  {
  };

  const int x, y, w;
  const bool starts_new_size;
};

rendering_sequence start_size(int total, int start_size, int sie);
rendering_sequence end_size(int total_points
```

4. Implement bookmarks using json
  Zoom to a bookmark

Autozoom:
- Can sometimes get lost

- [ ] When reusing an orbit, make sure that we update the number of iterations, particularly for a deep zoom

- [ ] Generate smooth animation to here
  - Zoom/calculate at the same time
  - UI shows a zoom state that's fully interpolated from the previous view
  - Try to get 60fps or something
  - Meantime, we're calculating the actual viewport on a separate viewport.
  - When we reach the end, we'll wait for rendering to finish
  - Then display the new viewport and carry on zooming.
- [ ] Lock to position.

Implementation:
- Enter a "zoom mode" whereby the saved viewport is used for all rendering.
- Parameters:
  - Start time, expected end time
  - Framerate
- Create a small buffer on expected end time based on current image.
- Store 3 viewports:
  1. The current image
  2. The zoomed image
  3. The computed image
- As soon as the end time is reached, wait until the image is rendered.

- [ ] Implement a "smooth zoom" button initially


- Renderer::continue_smooth_zoom(double time) // Every 16ms
  - Calculates the 


Autocolourmap:
- 2 modes: auto and manual


- [ ] Stop autozoom if the image depth is too low (e.g. 10)

- Create an array
- Pop from array when we don't have any more colours in the range
- Problem is that we've already rendered the pixels

Cm::set_pixels(double[])

Manual mode:
- Pass the array of pixels to the colourmap.
- Or, create an array of percentiles (100).







# Other

- [ ] Rendering glitches on power 3 mandelbrot

- [ ] Get a deeper series by looking more closely at the divergence criteria
- [ ] Get a deeper orbit by adding more terms?
- [ ] Auto algorithm gets stuck in a mandelbrot
- [ ] To auto-zoom to the edge, zoom to a point that's 10 higher than the mimimum.


- [ ] Center button doesn't work when you scroll

Bookmarks.json file.
- Store the radius in engineering form.
- Store numbers in 


Features:

- [ ] Better center and mandelbrot finder
- [ ] Auto-enhance colours
  How to make this a smooth/auto experience?

Create 100 buckets.
Sort the array and create the percentiles.

New colouring algorithm:
- Create an array with one RGB value per iteration.
- To sort it, 

Task: Find some very close minibrots? What is the closest they can come?




Interesting fact: All centers are on closed orbits, with a fixed period.
Minibrots are when orbits return to the same point over and over.
Look at where an interior point ends up.
We can find similar minibrots by simply following them one iteration.





- A "depth" histogram - allow us to see features at different depths.

- [ ] Delete the experiment, and instead repurpose the experiment for different view parameters.

- [ ] fractals:: namespace is stupid

- [ ] A few deep glitches, perhaps due to relative orbits?
- [ ] Bug: when we load a file we need to update the menus to select the correct item


- [ ] Render timings should also include the setup timing.

- [ ] Split up orbit.hpp

- [ ] Make it nicer to adapt Mandelbrot to Mandeldrop

- [ ] Surely we should colour "around" a point rather than just below/right

- [x] Don't aggressively normalize `high_exponent_real`
- [ ] Update all the other fractals to use new data types
- [ ] Too many artefacts when scaling and zooming.
  Instead, the "view" contains a pixel size.
  When the pixels in the view are larger than the new pixels, replace them.
  Still figure out
- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.

- [ ] Reuse orbit and calculate new reference orbit async 

Rendering: We keep a view stored (as doubles), together with its coordinates and pixel size.
When we zoom, we first display the outer window to the required scaling.
As soon as we get data on the inner zoom, we choose whether to replace the current view with the data.

Limit scrolling to wait for rendering to catch up?

- [ ] Multiple top-level windows
- [ ] Look at time since we rendered last plane

- [ ] Re-test very deep zooms





- [ ] Understand reference orbits better
  - What is the "orbit utilization"
  - What percentage of iterations
  - When is it cheaper to compute a new reference orbit?
  - Can we create a reference of a reference?
  - Can other reference orbit "split" from the old one?

- [ ] Can we "skip forward" n iterations? Is there a closed form for that??? Can we approximate the "skip forward" action using its own Taylor series??
  - What about skipping epsilon forward?

Basically it's the depth of range that causes big problems. Computing up to the minimum is usually fine.

- [ ] Speed up relative_orbit when using `high_exponent_number` for deep zooms

- [ ] Colour 122 is nice

- [ ] Refactor mandelbrot parameters, for example use real_number.
Maybe there's a hard-coded way to express the delta range? So we can scale all deltas by M, e.g. bias them all by 500.

Optimization:
- [ ] Be less strict about rejecting invalid iteration counts
- [ ] Look into deeper zooms
- [ ] Visualize skipped iterations. Could be a clue into where to place the reference orbit


## Implement a bookmarks feature.
Have a saved library of interesting fractals to look at.

```
struct library_item
{
  std::string section, name, description, algorithm, x, y, r;
  int colour_scheme;
  double colour_gradient;
};
```

- [ ] Auto-enhance image if it's too dim - the range is unsuitable

- [ ] Set the center orbit to be on the Mandelbrot for the best precision
  E.g. off-center, average 85760 skipped iterations
  Better center, average 

- [ ] Load file to change the menu


- [ ] Write-up of Taylor series

Short term goals:
- [ ] MSIX installer

Ideas:
- [ ] `[` and `]` keys to change the gradient
- [ ] Instant recolour
- [ ] Performance improvements

Blockers:
- [ ] Windows installer
- [ ] Building Qt from source on Windows still does not work

Bugs:
- [ ] Can zoom out to radius 3
- [ ] Still some zoom inaccuracy on Mandeldrop around 1e-20

Colouring:
- [ ] Find a new default seed
- [ ] Enhance contrast algorithm by sorting all of the pixels
  - Keep the array of doubles instead of throwing them away
  - Split the
  - Each time we reach a new depth, we append to the "depths" array.
  - Each time we fail to reach a depth, we'll pop from the "depths" array
- Create a new colourMap class that handles this.
- Auto gradient - each time we reach a new zoom level, we put the new pixels in a different gradient?

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

# Log term tasks

- benchmarks
- tests
- Tidy up the installer text and license
- [ ] Progress bar

High precision tidy:
- Express size in bits
- Configure integer size as well

Less important tasks:
- [ ] Smooth zoom
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
