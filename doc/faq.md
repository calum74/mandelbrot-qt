# Mandelbrot-Qt Frequently Asked Questions

## What is Mandelbrot-Qt written in?

C++.

## What depths can it reach?

The current implementation is limited to a radius of around 1e-1000, which requires 640 bits of storage per number.

## What arithmetic library do you use?

There are three types of number used:

1. A low precision low exponent complex number
2. A high precision low exponent complex number
3. A high exponent low precision complex number

## How do you get the funky rendering?

When zooming in or out, the current image is stretched pixel by pixel. This can sometimes introduce unpleasant artefacts. On top of that, each pixel stores an "error" which is used to decide whether to replace a pixel with an interpolated version. The error is taken to be the Manhattan distance to the interpolated pixel, giving a pleasant triangulation effect. It was discovered by accident and I liked it.

## How do you calculate the depth (bailout)?

The new bailout is twice the depth of the 99th percentile depth of the current image. This usually gives sufficient head-room to not lose any detail on mini-brots.

## What calculation algorithms do you use?

Basically it's as far as Taylor series expansion as described on Wikipedia.

Of course, the Wikipedia article leaves plenty to be inferred. On particular thing I learned was that 

## How are the colours generated?

The colourmap is essentially random

## How do you find mini-mandelbrots (mini-brots?)

In the regular Mandelbrot set, minibrots have a rotational symmetry of 2, so look for any region that is symmetrical. You may need to go deep, but the mini-brot will be there.

## What are your future plans?

See the list of PRs for future ideas.