# Task list

- [ ] Don't aggressively normalize `high_exponent_real`
- [ ] Update all the other fractals to use new data types
- [ ] Too many artefacts when scaling and zooming.
  Instead, the "view" contains a pixel size.
  When the pixels in the view are larger than the new pixels, replace them.
  Still figure out
- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.
  
- [ ] Reuse orbit and calculate new reference orbit async 

## Performance improvements

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
