# Welcome to Mandelbrot-Qt

[![CMake on multiple platforms](https://github.com/calum74/mandelbrot-qt/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/calum74/mandelbrot-qt/actions/workflows/cmake-multi-platform.yml)

MandelbrotQt is a desktop application to explore the Mandelbrot set. If you just want to look at fractals, go to the [releases](https://github.com/calum74/mandelbrot-qt/releases) page.

## Running the software

To install binaries for your plaform, just go to the [releases](https://github.com/calum74/mandelbrot-qt/releases) page. MandelbrotQt has binaries for MacOS, Windows, and Linux.

## How to use it

You will be presented with a view of the Mandelbrot set. You can navigate around this image by zooming using the mouse wheel (or touchpad), and you can pan/scroll by dragging the mouse. You can go very "deep", up to around 10e-300, and high "depth" of over 1 million iterations.

There will be some regions will will take longer to render, and this is generally caused by a high number of iterations, or in areas are highly complex and close to black regions. This is not a bug.

The menus contain other useful options:

* `R` to recolour the palette, if you want to see the image using different colours, or if the current colour scheme does not work well.
* `E` to enhance the palette to increase the contrast based on the number of iterations in the current image. When you zoom out again, you might need to hit `E` again
* To save an image, use copy and paste, or hit `S` to perform a quick-save of the current image to your desktop. There is no option to change the resolution, but you can resize the window.
* To see the current coordinates, select "Go to..." from the menus, which tells you your current coords and allows you to set them.

Auto-bailout: MandelbrotQt implements a heuristic to estimate the maximum number of iterations. If this is too low, then the image can contain excess black areas. You can disable auto-bailout by disabling "Automatic depth" option in the menu, and use `I` and `O` to increase or decrease the number of iterations.

## What is a Mandelbrot set?

In brief, the Mandelbrot set is a type of "fractal", a mathematical object of unlimited complexity. The Mandelbrot set is a particularly interesting and beautiful fractal, and is in my opinion unrivalled.

## What to do

- Find minibrots - minature Mandelbrot sets embedded in the original image. There are an unlimited number of these. To find minibrots, observe an area with a rotational symmetry, which as you zoom in will resemble a cross, star, and circle. Carry on zooming right to the center.
- What is the smallest minibrot you can find?
- Make sure to explore all the different regions.

## Contributing to MandelbrotQt

Contributions are very welcome! See [CONTRIBUTING.md] for further information.

If you just want to learn how it works, check out the [implementation notes](docs/implementation.md).
