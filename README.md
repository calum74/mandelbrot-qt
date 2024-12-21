# Welcome to Mandelbrot-Qt

[![CMake on multiple platforms](https://github.com/calum74/mandelbrot-qt/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/calum74/mandelbrot-qt/actions/workflows/cmake-multi-platform.yml)

*Insert picture here*

MandelbrotQt is a desktop application to explore the Mandelbrot set. If you just want to look at fractals, go to the Downloads section.

I wrote it because there didn't seem to be many good options for MacOS, and also because it's a fun learning experience.

## Running the software

To install binaries for your plaform, just go to the relevant [releases] page. MandelbrotQt has binaries for MacOS, Windows, and Linux.

## Building the software


# What is the Mandelbrot set?


# Background

The Mandelbrot set is a type *fractal* - a mathematical shape of unlimited complexity. The deeper you look at it, the more detail emerges. This behaves very differently to a regular shape or graph, where  when you zoom in far enough, it starts to look like a straight line.

The Mandelbrot set is defined as the set of points c that don't escape to infinity under the transformation z->z+c. In spite of the simplicity of this formula, it turns out that points arbitrarily close together can behave completely differently, a phenomenon known as the "butterfly effect". Even small variations can become amplified to the degree that given enough time, the outcome is totally different. So named, because when a butterfly beats its wings, it can eventually trigger a hurricane on the other side of the earth.

Another complexity, ehem, is that the Mandelbrot set is defined on the complex numbers. Each point in the Mandelbrot is actually on the complex plane, and is represented by x+iy. For the purposes of calculation however, we can ignore the fact that we are dealing with complex numbers, and just treat each point as the pair (x,y).

Drawing a basic Mandelbrot set is not too complicated, but there is a lot more work required to make this a great experience. The first problem is that the algorithm never terminates, so we need to implement a "cut-off" and stop iterating after a certain number of iterations.

