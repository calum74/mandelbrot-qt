# Task list

BLA ideas:
1. Check the actual size of the error - do the maths!
  What's the maximum norm(delta) at each step? Record this?
  Ensure that individual steps don't go wrong by being close to 0.
2. Think about batching jumps into steps of 512.
    Does this give a better utilization?
    Store an A_i_n and B_i_n in each entry
3. Find a way to report the orbit reuse
   > Developer features
   - Switches on verbose output
   - Show additional mandelbrots:
    - Experimental
    - Reference MB
    - Naive MB

Radius 5-21 completed in 0.12 seconds, depth 159-1279, skipped 1000/1200 (80%)

Stage as three algorithms:
1. Linear coefficients, with cached single-orbit translation
2. Bilinear coefficients, with a fixed step size (e.g. 100)
3. Hybrid: Linear + adaptive step sizes.

Why it works:
- We can efficiently translate our reference orbit since it turns out that all BLA coefficients are the same within their region of validity. We tend to render adjacent pixels together, which means that when we render an adjacent pixel, we use the BLA coefficients for a reference orbit that's adjacent to the pixel we are calculating.

**Theorem 1:** (Fundamental theorem of BLA)

If $|\epsilon_i| < \epsilon|z_i|$ and $ |\epsilon_i|^2 < \frac{\epsilon|\delta|}{2}$ then BLA is valid.

Proof: $\epsilon_{i+1} = 2\epsilon_iz_i + \epsilon_i^2 + \delta \approxeq 2\epsilon_iz_i + \delta$

$\iff |\epsilon_i^2| < \epsilon|2\epsilon_iz_i + \delta|$

$\impliedby |\epsilon_i^2| < \epsilon|\epsilon_iz_i|$ and $ |\epsilon_i^2| < \frac{\epsilon|\delta|}{2}$

$\iff |\epsilon_i|^2 < \epsilon|\epsilon_i||z_i|$ and $ |\epsilon_i|^2 < \frac{\epsilon|\delta|}{2}$

$\iff |\epsilon_i| < \epsilon|z_i|$ and $ |\epsilon_i|^2 < \frac{\epsilon|\delta|}{2}$

$\square$

**Theorem 2:**

If
$|\delta| < \frac{\epsilon|z_i|}{|B_i|}$ and $|\delta| < \frac{\epsilon}{2|B_i|^2}$
then the linear approximation $\epsilon_i \approxeq B_i\delta$ is accurate.

Proof: $\epsilon_i \approxeq B_i\delta$

$\impliedby |\epsilon_i| < \epsilon|z_i|$ and $ |\epsilon_i|^2 < \frac{\epsilon|\delta|}{2}$

$\iff |B_i\delta| < \epsilon|z_i|$ and $ |B_i\delta|^2 < \frac{\epsilon|\delta|}{2}$

$\iff |B_i||\delta| < \epsilon|z_i|$ and $ |B_i|^2|\delta| < \frac{\epsilon}{2}$

$\iff |\delta| < \frac{\epsilon|z_i|}{|B_i|}$ and $|\delta| < \frac{\epsilon}{2|B_i|^2}$

$\square$

Let's instead look at the cumulative error $E_i$

$E_i = z_i - B_i\delta$



Theorem 2 gives us a maximum value of $|\delta|$ at each iteration.

Do we need to track this for every iteration, which we can implement by storing a $|\delta|_{max,i}$ for each iteration, and ensuring that $|\delta|_{max,i} = min(|\delta|_{max,i-1},\frac{\epsilon|z_i|}{|B_i|},\frac{\epsilon}{2|B_i|^2})$, or is this condition already baked into $B_i$?

Implementation: Every 512 steps, reset the jump-forward.

**Theorem 3:** (Validity of BLA coefficients)

If

$|A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
${|A_i|} < \frac{2\epsilon}{|\epsilon_j|\sqrt{|B_i|}}$ and
${|A_i|} < \frac{2\epsilon}{|\epsilon_j||B_i|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

then 

$\epsilon_{i} \approxeq A_i\epsilon_j + B_i\delta$

Proof:

$\epsilon_{i} \approxeq A_i\epsilon_j + B_i\delta$

$\impliedby |\epsilon_i| < \epsilon|z_i|$ and $ |\epsilon_i|^2 < \frac{\epsilon|\delta|}{2}$

$\iff |A_i\epsilon_j + B_i\delta| < \epsilon|z_i|$ and $|A_i\epsilon_j + B_i\delta|^2 < \frac{\epsilon|\delta|}{2}$

$\impliedby |A_i\epsilon_j| + |B_i\delta| < \epsilon|z_i|$ and $|A_i\epsilon_j + B_i\delta| < \sqrt\frac{\epsilon|\delta|}{2}$

$\impliedby |A_i\epsilon_j| + |B_i\delta| < \epsilon|z_i|$ and $|A_i\epsilon_j| + |B_i\delta| < \sqrt\frac{\epsilon|\delta|}{2}$

$\impliedby |A_i||\epsilon_j| + |B_i||\delta| < \epsilon|z_i|$ and $|A_i||\epsilon_j| + |B_i||\delta| < \sqrt\frac{\epsilon|\delta|}{2}$

$\impliedby \frac{|A_i||\epsilon_j|}{2} < \epsilon|z_i|$ and $\frac{|B_i||\delta|}{2} < \epsilon|z_i|$ and 
$\frac{|A_i||\epsilon_j|}{2} < \sqrt\frac{\epsilon|\delta|}{2}$
and $\frac{|B_i||\delta|}{2} < \sqrt\frac{\epsilon|\delta|}{2}$

$\iff |A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
${|A_i|^2}|\epsilon_j|^2 < 2\epsilon|\delta|$ and
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

$\iff |A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
${|A_i|^2} < \frac{2\epsilon|\delta|}{|\epsilon_j|^2}$ and
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

$\impliedby |A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
${|A_i|^2} < \frac{2\epsilon}{|\epsilon_j|^2}\frac{2\epsilon|z_i|}{|B_i|}$ and
${|A_i|^2} < \frac{2\epsilon}{|\epsilon_j|^2}\frac{2\epsilon}{|B_i|^2}$ and
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

$\iff |A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
${|A_i|^2} < \frac{4\epsilon^2}{|\epsilon_j|^2|B_i|}$ and
${|A_i|^2} < \frac{4\epsilon^2}{|\epsilon_j|^2|B_i|^2}$ and
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

$\iff |A_i| < \frac{2\epsilon|z_i|}{|\epsilon_j|}$ and
$|\delta| < \frac{2\epsilon|z_i|}{|B_i|}$ and 
${|A_i|} < \frac{2\epsilon}{|\epsilon_j|\sqrt{|B_i|}}$ and
${|A_i|} < \frac{2\epsilon}{|\epsilon_j||B_i|}$ and
$|\delta| < \frac{2\epsilon}{|B_i|^2}$

$\square$

This does not help at all!

- Bookmarks flags can appear in the wrong place when starting a zoom in

- Tidy up bookmarks
- Maybe sort them
- Plant flags on all the bookmarks.
  - Size of icon relates to distance
    Return the difference of the logs of the radius.
    If negative, hide it.
    Size = k * log(size_ratio)
  - Don't show bookmark if we've gone past it
- Use openmp

Bugs:
- [ ] Multiple toplevels don't sync bookmarks
  Menus not updated properly
- [ ] Zoom in goes too far
- [ ] Display the time without setprecision(2)??
- [ ] Bugs in New Year fractal. (imprecision of some kind)
- [ ] Glitches in Magnus Opus Ex
- [ ] Animate to here still carries on zooming mysteriously
  I think we need to cancel the current calculation as it only happens when there is still a rendering going on?
- [ ] Suddenly a nasty judder whilst rendering.
- [ ] Scrolling disrupts depth calculations
- [ ] Smooth zoom stops now if there's not enough pixels to update the depth

Improvements:
- [ ] Implement a reference Mandelbrot (perturbation only)
- [ ] Lock window
- [ ] Karatsuba multiplication
- [ ] Carry on animating during window resizing
- [ ] Maybe load the bookmarks async??
- [ ] Comparative MB sets - can we look at 2 regions side-by-side?
  - New toplevel window
- [ ] Better filename for bookmarks.json
- [ ] Zoom step is always fast?
- [ ] Continuous zoom is always smooth
- [ ] Eta/ progress indicator.
- [ ] Smoother zoom out??
- [ ] GPL license
- [ ] Esc to stop animations
- When we scroll, we'll sometimes lose pixels
- Sometimes shows finished calculation when it's actually calculating something new
  - Ensure that "calculating" takes priority
  - Depth 0-0 looks silly
- Better normalization logic for high_exponent_real
- On open, resume previous place visited?
- [ ] "1 CPU core, "4 CPU cores", "All CPU cores"
- [ ] Rename "oversampling" to "High definition"
- [ ] Better gradient enhancements
- [ ] Shading
- [ ] Import/export bookmarks?
- [ ] Show number of iterations skipped
- [ ] Put a "flag" icon next to each bookmark
- [ ] Dynamic length of number?

BLA:
- What's the imprecision on our epsilon? It may be that we can actually manage a much longer sequence in a branch, as we are only dealing with a worst-case.
- What did the terms look like in our "shadow tree" underneath our current branch that we didn't actually calculate?

For our shadow tree, we can calculate our distance to the reference orbit $\Epsilon'_i$ using the initial branch. We obviously have our $\Delta$ relative to the first branch. 

Then for a point around the orbit at $z''_i = A'_{j,i-j}\epsilon'_j + B'_{j,i-j}\delta = A_{j,i-j}\epsilon_j + 


Ideas: 
- If the Taylor series is valid, we can find a delta which makes the orbit coincide with the starting point.
- Using Newton Raphson.
- Can we approximate "32 steps" using perturbation theory?
  - Only for low epslions.
- [ ] Class OrbitConfiguration which contains all of the parameters for the orbit

# Mandelbrot explorer
Where mathematics meets art


Classification project:
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
- [ ] Sometimes dots remain in black areas after a long calculation.

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

- [x] Surely we should colour "around" a point rather than just below/right

- [ ] Update all the other fractals to use new data types
- [ ] Rendering sequence to literally map integers to coords and sizes.
  Make it an O(1) operation.


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
- [x] Dealing with very deep zooms. E.g. scaledSmallNumber  
- [x] Would adding another Taylor series term help?
- [ ] Can we partially evaluate the Taylor series??
- [ ] Does choice of reference orbit matter?
- [ ] Can we combine 2 reference orbits?
- [ ] Can we translate a Taylor series efficiently?

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
