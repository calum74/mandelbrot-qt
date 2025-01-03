# Contributing to Mandelbrot and MandelbrotQt

How to get in touch: Typically [open an issue](https://github.com/calum74/mandelbrot-qt/issues/new). You can also find me as `@drcalum` on Bluesky.

Contributions are very welcome! To contribute to MandelbrotQt, typically create a pull-request with your changes. There are other ways to contribute:

- Filing bug reports
- Asking questions
- Suggestions for new features
- Reviewing pull requests.

When creating a PR, bear in mind,

- Adhere to existing conventions
- Where possible, create unit tests or benchmarks that demonstrate the fix. If something is just an aesthetic improvement or a theoretical improvement, it might get rejected.

## Building the software

Trying to build anything with Qt is an utterly miserable and demoralising experience. MandelbrotQt using C++ and Qt, and is built using CMake.

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

Install Qt from https://www.qt.io/download-qt-installer-oss. If you install QtCreator and build the project

You can also use QtCreator, Visual Studio, Visual Studio Code to open the CMake project.

## Creating an installer

The Actions workflows do create an installer, but this is only using the shared/dynamic Qt toolchain. We actually want to use the static Qt toolchain when distributing MandelbrotQt.

On Mac: Download the source code using the Qt installer https://www.qt.io/download-qt-installer-oss (e.g. version 6.8.1). 

```
./configure -static
cmake --build . --parallel
cmake --install .
```

Then, reconfigure cmake to use the static Qt build

```
mkdir mandelbrot-qt-static
cd mandelbrot-qt-static
cmake ../mandelbrot-qt -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local/Qt-6.8.1
make
```

On MacOS: `cpack -G DragNDrop`