# Task list

Bugs:
- [ ] Glitch on main mandelbrot when resizing window sometimes
- [ ] When resize, why is there so much black in the way?

Scale palette:
- Reset when we go home?
- When we zoom out partially, still update view_min etc

Enhancements:
- Display the gradient somewhere
- When saving images, also save a log of coordinates

Documentation and tidy:
- [ ] Implementation notes
- [ ] Further code tidy
- [ ] More unit tests, for example, orbits
  - Mandelbrot - list the classes and document them

Create issues:
- [ ] Set windows icon
- [ ] Icon on Mac isn't rounded
- [ ] Installer does not install start menu item on Windows
- [ ] Add fractals
- [ ] Speed up calculation
- [ ] Remove glitches
- [ ] Create benchmarks

- [ ] Make sure to not recalculate calculated pixels (e.g. scroll)

Future:
- [ ] Export image
- [ ] File load/save
- [ ] Generate movie

Mandeldrop bug:
- Looks like there's an imprecision at -0.0000036023362057037546653,0.7137320572149256883936381,0.7137320572149256883936381,0.7137320572149256883936381

- [ ] Implement general Taylor series expansion


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
