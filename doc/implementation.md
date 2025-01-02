# Implementation of MandelbrotQt

## Introduction

Why do we need yet another viewer of the Mandelbrot set? Well, the venerable Kalles Fractaler didn't run on Mac, and looks to be in maintenance mode for now. I just couldn't find a browser for the Mandelbrot set that I was happy with, so I wrote my own.

I wanted something that was extremely intuitive to use.

## Code layout

The code is split across 2 repos: 

- https://github.com/calum74/mandelbrot contains the basic algorithms and mathematics
- https://github.com/calum74/mandelbrot-qt contains an app to render and navigate

...

## Underlying maths

The mathematics of the Mandelbrot set is quite simple, and just counts the number of iterations of the formula (z -> z*z+c) for each point c on the complex plane. Provided you understand complex numbers, it's not a big deal.

There are some extra pieces of maths to understand however:

1. Perturbation theory. In order to get deep views beyond about 10^-14, you'll need higher-precision numbers, because the hardware based 64-bit `double` datatype just lose precision. Fortunately, you can rearrange the equation for the mandelbrot set to calculate the distance from a "reference orbit" (calculated using higher precision numbers), and compute deltas using regular `double` numbers. This drastically speeds up calculation.
2. High precision numbers. These are needed to compute reference orbits. An off-the shelf library for high precision numbers (like GMP) would be an excellent choice, but I rolled my own.
3. Taylor series approximations. For the points around a reference orbit, the distance from the reference orbit can be approximated by a Taylor series. Since delta is typically very small, the Taylor series converges very rapidly. This allows us to use a binary search to count escape iterations, instead of a linear search.
4. Smooth colouration. A nice aesthetic enhancement is to look at the distance of the escaped orbit in order to give a fractional iteration count.

To be clear, I didn't invent any of these ideas.

## Smooth scaling

As well as making the underlying calculation as fast as possible, we'll want to be as smart as possible about rendering the image. MandelbrotQt prioritises smooth speed and navigation over image quality.

- When zooming, we'll scale the current image
- When panning, we'll scroll the pixels
- When computing, we compute the image in 16x16, 8x8, 4x4, 2x2 and 1x1 tiles, so that we can display something to the user without waiting for every pixel to be computed.
- When a tile has 4 identical corners, we avoid computing the center of the tile. This is technically an imprecision but it works fine in practise.
- Calculations must be interruptible with a cancellation token, implemented using `std::atomic<bool>&`. Slow algorithms must check the value of this token frequently so that the current calculation can be abandoned quickly.

Pixels store an "error" (actually hacked into the upper 8 bits of each pixel). When the error is 0, it means we don't need to recompute that pixel.

The class `rendering_sequence` returns a sequence of (x,y) coordinates to calculate.

## Auto-depth

To avoid having to manually tweak the depth (bailout/maximum iteration count), I use a heuristic to look at the distribution of depths in the current image.
Then, look at the 99.9 percentile depth, which is close to the maximum depth but allows a small number of points to be excluded, and then double this (see AsyncRenderer::discovered_depth()`)to get the new bailout.

Computing the 99.9 percentile is found in `percentile.hpp`.

## Orbits library

## Mandelbrot calculation library

## High precision arithmetic library

