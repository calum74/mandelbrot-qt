# Task list
    
- Investigate magic
Either the epsilon is wrong, or the terms are wrong. Or an offset is wrong.





- Investigate clusters

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

## Performance improvements

The reason for poorer performance is because the reference orbit is too far away. This becomes more critical in complicated and deep regions, where the series can lose precision quite quickly, resulting in the number of saved iterations diminishing rapidly.

The solution is to create a new reference orbit close to the point being calculated. Rather than compute the high precision orbit itself, we can compute a relative orbit using perturbation theory, relative to (say) the orbit in the center of the image.

Sometimes the new orbit needs to be computed using high precision numbers because the original reference orbit runs out of precision (or, can it be restarted using the trick???).

We can also compute Taylor series terms for the new reference orbit, so that the new reference orbit can be used to compute nearby points.

As soon as the performance of the current orbit starts to deteriorate, given by the fact that it lost precision relative to the number of iterations, we can use this fact to start another reference orbit.

The calculation is that it's cheaper to create a new reference orbit than to carry on iterating on a sub-optimal orbit that's too far away.s

## Orbital management

The problem is in finding the best orbit, and managing this in a multi-threaded setting.



- [ ] Go absurdly deep once finished. E.g. 1e-10000.z


The solution is to create a `taylor_series_orbit` from a relative orbit.


Idea: We want to create multiple reference orbits.
1. Create one high precision orbit
2. Create multiple reference orbits from the high precision orbit
3. Generate series for each of these relative orbits.
4. Detect when the current orbit is underperforming, and switch to a new orbit




- [ ] Understand reference orbits better
  - What is the "orbit utilization"
  - What percentage of iterations
  - When is it cheaper to compute a new reference orbit?
  - Can we create a reference of a reference?
  - Can other reference orbit "split" from the old one?

- [ ] Can we "skip forward" n iterations? Is there a closed form for that??? Can we approximate the "skip forward" action using its own Taylor series??
  - What about skipping epsilon forward?

- [ ] Ability to change algorithms and not reset the coordinates (Needed to check validity of certain assumptions).

Basically it's the depth of range that causes big problems. Computing up to the minimum is usually fine.

- [ ] Multiple reference orbits - pick the one that gives the best depth.
- When the orbit underperforms, consider adding a new orbital calculation

- [ ] Speed up relative_orbit when using `high_exponent_number` for deep zooms

- [ ] Colour 122 is nice

- [ ] Refactor mandelbrot parameters, for example use real_number.
Maybe there's a hard-coded way to express the delta range? So we can scale all deltas by M, e.g. bias them all by 500.

- Use concepts for
  `CReal`
  `CComplex`

Optimization:
- [ ] Be less strict about rejecting invalid iteration counts
- [ ] Look into deeper zooms
- [ ] Visualize skipped iterations. Could be a clue into where to place the reference orbit

Reuse the reference orbit from the previous calculation as there's no need to recalculate it every time we zoom??

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

Bugs:
- [ ] When resize, why is there so much black in the way?

Enhancements:
- [ ] Display the gradient somewhere
- [ ] Create benchmarks
- [ ] Make sure to not recalculate calculated pixels (e.g. scroll)
- [ ] Implement general Taylor series expansion
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
