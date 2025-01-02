# Task list

Documentation and tidy:
- [ ] Implementation notes
- [ ] Further code tidy
- [ ] More unit tests, for example, orbits
  - Mandelbrot - list the classes and document them

Papercuts:
- [ ] Resizing the window displays garbage
- [ ] Icon on Mac
- [ ] Installer does not install program on Windows
- [ ] Make sure to not recalculate calculated pixels

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
