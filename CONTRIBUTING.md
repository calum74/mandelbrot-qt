# Contributing to Mandelbrot and Mandelbrot-Qt

How to get in touch: Typically [open an issue](https://github.com/calum74/mandelbrot-qt/issues/new). You can also find me as `@drcalum` on Bluesky.

Contributions are very welcome! To contribute to Mandelbrot-Qt, typically create a pull-request with your changes. There are other ways to contribute:

- Filing bug reports
- Asking questions
- Suggestions for new features
- Reviewing pull requests.

When creating a PR, bear in mind,

- Adhere to existing conventions
- Where possible, create unit tests or benchmarks that demonstrate the fix. If something is just an aesthetic improvement or a theoretical improvement, it might get rejected.

## Prerequisites

You will need Qt, C++ and CMake, which should work on Linux, Mac and Windows.

On Windows: The simplest way to get Qt installed is via https://www.qt.io/download-qt-installer-oss.

On Linux:

```
sudo apt install git clang cmake qt6-base-dev libxkbcommon-dev
```

On Mac:

```
brew install cmake qt6
```

## Building Qt from source

## Downloading the source code

## When Qt breaks

This happens a lot.



## Building the software

Trying to build anything with Qt is an utterly miserable and demoralising experience. Mandelbrot-Qt using C++ and Qt, and is built using CMake.

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

If it fails (which happens around 70% of the time, thanks trolls), you will have to build Qt yourself locally.

On Windows/Mac:

Install Qt from https://www.qt.io/download-qt-installer-oss. If you install QtCreator then it should be able to just open your project (`CMakeLists.txt`) and build it.

Set the prefix path to something like:

```
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2022_arm64\lib\cmake\Qt6  -DCMAKE_BUILD_TYPE=Release
```

If CMake fails, try running the specific generator for your platform, e.g. `C:\Qt\6.8.1\msvc2022_64\bin\qt-cmake`, or `/usr/local/Qt-6.10.0/bin/qt-cmake ../mandelbrot-qt -DCMAKE_BUILD_TYPE=Release` for example.

On Mac:

Homebrew also contains Qt, and you will need `brew install cmake qt`.

## Creating an installer

The Actions workflows do create an installer, but this is only using the shared/dynamic Qt toolchain. We actually want to use the static Qt toolchain when distributing Mandelbrot-Qt.

On Mac: Download the source code using the Qt installer https://www.qt.io/download-qt-installer-oss (e.g. version 6.8.1). 

Or, `git clone https://github.com/qt/qtbase.git`, followed by

```
./configure -static
cmake --build . --parallel
sudo cmake --install .
```

Then, reconfigure cmake to use the static Qt build,

```
mkdir mandelbrot-qt-static
cd mandelbrot-qt-static
cmake ../mandelbrot-qt -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local/Qt-6.8.1
make
```

On MacOS: `cpack -G DragNDrop`
