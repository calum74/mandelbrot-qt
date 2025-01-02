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
* To save an image, use copy and paste. (Image export option might be implemented in version 2 if there is sufficient interest.)
* Other Mandelbrot-based fractals.

## What is a Mandelbrot set?

In brief, the Mandelbrot set is a type of "fractal", a mathematical object of unlimited complexity. The Mandelbrot set is a particularly interesting and beautiful fractal, and is in my opinion unrivalled.

## What to do

- Find minibrots - minature Mandelbrot sets embedded in the original image. There are an unlimited number of these. To find minibrots, observe an area with a rotational symmetry, which as you zoom in will resemble a cross, star, and circle. Carry on zooming right to the center.
- What is the smallest minibrot you can find?
- Make sure to explore all the different regions.


## Building the software

Trying to build anything with Qt is an utterly miserable and demoralising experience.

On Linux:

```
sudo apt install git clang cmake qt6-base-dev
git clone https://github.com/calum74/mandelbrot-qt --recursive
mkdir mandelbrot-qt-build
cd mandelbrot-qt-build
cmake ../mandelbrot-qt -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```

On Windows/Mac:

Install Qt from https://www.qt.io/download-qt-installer-oss. 

You can also use QtCreator, Visual Studio, Visual Studio Code to open the CMake project.

## Creating an installer

On Mac: Download the source code using the Qt installer https://www.qt.io/download-qt-installer-oss (e.g. version 6.8.1). 

```
./configure -static
cmake --build . --parallel
cmake --install .
```

Then, reconfigure cmake to use the static build

```
mkdir mandelbrot-qt-static
cd mandelbrot-qt-static
cmake ../mandelbrot-qt -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local/Qt-6.8.1
make
```

On MacOS: `cpack -G DragNDrop`
