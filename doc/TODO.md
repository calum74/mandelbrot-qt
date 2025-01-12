# Task list

Center-finding
- C and Z buttons

Smooth zoom. When we stretch the view, try to avoid stretching artefacts.
Perhaps don't show pixels at all resolutions?

We take the "average x", weighted by depth.



Implement a bookmarks feature.
Have a saved library of interesting fractals to look at.


Center-finding:
- Just add the depths on each row and column.
- Pick the row or column with the highest total depth.
- Maybe average it by 

- [ ] Implement "biassed reals"
- [ ] Refactor mandelbrot parameters, for example use real_number.
Maybe there's a hard-coded way to express the delta range? So we can scale all deltas by M, e.g. bias them all by 500.




```
struct library_item
{
  std::string section, name, description, algorithm, x, y, r;
  int colour_scheme;
  double colour_gradient;
};
```
Use gradient to point to the center.
`C` button shifts image to the center if possible.
`Z` button zooms and centers.
- Just look at the very deepest parts
- Or look at the gradient

Auto-enhance image if it's too dim - the range is unsuitable


Next steps:
- Deep zooms e-550 have glitches near mandelbrots. e.g. fractal60, fractal66.
  - This could be another precision glitch. Is this from the Taylor series? (And how to fix???) When calculating epsilon, do I need to combine high_precision_real and high_exponent_real in some way?

- Need a better abstraction for "size" and "precision" of a number, and just create traits

- Think about whether we can use an int_64 as the main part of the high_exponent_real

- Use concepts for
  `CReal`
  `CComplex`

- Optimize the terms calculation a bit. Can get a closed form for the fourth term?

Reuse the reference orbit from the previous calculation as there's no need to recalculate it every time we zoom??

Optimization:
- [ ] Be less strict about rejecting invalid iteration counts
- [ ] Look into deeper zooms
- [ ] Visualize skipped iterations. Could be a clue into where to place the reference orbit

- [ ] Use previous position as a starting-point for the skip-iterations
- [ ] Zoom beyond 10e-300
- [ ] Set the center orbit to be on the Mandelbrot for the best precision
  E.g. off-center, average 85760 skipped iterations
  Better center, average 

- [x] Imprecision at 10e-150 in Fractal 42.



- [ ] Load file to change the menu


- [ ] Only use the exponented real for the Taylor series coefficients, but the regular iteration is with doubles.
- [ ] New time has doubled, so look at that.
  Why are different number of iterations skipped??
- [ ] Write-up of Taylor series

Plan for today:
1. Log the maximum precision in the reference orbit
2. Devise a higher precision `double`. For example

- [ ] Gradient calculation looks to be a bit wrong and gives a blank screen. "Enhance" is broken

```c++
template<typename Real, typename Exp>
class higher_precision_real
{
public:

private:
  Real value;
  Exp exponent;
};
```

Better heuristics on imprecision. Still could get glitches on cubic which only appear on 4 terms.
Glitch in Fractal29
Performance problems at very deep zooms - are we running out of precision in the Taylor series terms
Need a higher precision number
- [ ] Detect maximal valid entry


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
- [ ] Currently limited to 1e-261 radius
- [ ] Looks like the wrong resolution is chosen
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

Today:
- [ ] Windows machine with static Qt
  - Use cmake-qt 
- [ ] Create a new release.

Bugs:
- [ ] Glitch on main mandelbrot when resizing window sometimes
- [ ] When resize, why is there so much black in the way?

Enhancements:
- [ ] Display the gradient somewhere
- [ ] Create benchmarks
- [ ] Make sure to not recalculate calculated pixels (e.g. scroll)
- [ ] Implement general Taylor series expansion

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
