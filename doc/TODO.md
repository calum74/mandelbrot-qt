# Task list

Short term goals:
- Currently limited to 1e-261 radius
- [ ] class ViewParameters
- [ ] File save format needs versioning and ability to store colour palette and gradient

- [ ] Ensure that the colour scheme is consistent between platforms. (default_rng may be wrong).
- [ ] Find a new default seed
- [ ] Enhance contrast algorithm by sorting all of the pixels
  - Keep the array of doubles instead of throwing them away
  - Split the
  - Each time we reach a new depth, we append to the "depths" array.
  - Each time we fail to reach a depth, we'll pop from the "depths" array
- Create a new colourMap class that handles this.

```c++
// We create a variable gradient which
// attempts to smooth out the gradient between low- and high- gradient areas
// based on grouping points into equally-sized bins.
class ProportionalColourMap
{
  void startImage()

  RGB lookup(double d) {
    points.push_back(d);

    // How to perform the lookup???
  }

  void recalibrate() override
  {
    // Perform colouration based on the previous image, ready for the next image
  }

  void incremental_calibrate() override
  {
    // End of image, so we can adjust our calibration to higher depths
  }
};
```


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
