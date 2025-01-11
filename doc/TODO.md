# Task list

- [ ] Use previous position as a starting-point for the skip-iterations
- [ ] Zoom beyond 10e-300
- [ ] Be less strict about rejecting invalid iteration counts
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
