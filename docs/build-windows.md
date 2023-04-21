The recommended way (i.e. what its developer does) to build Crispy Doom on Windows is to use the MinGW-w64 compiler in an [MSYS2](https://www.msys2.org/) environment. Follow these steps for compiling 32-bit binaries with GCC (some steps may differ for other build targets, e.g. you'll have to replace all instances of `w32` with `w64` and `i686` with `x86_64` in steps 2, 4 and 5 in case you want to do a 64-bit GCC build):

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MINGW32 Shell** (`mingw32.exe`) and update the package list:
```
pacman -Sy
pacman -Suu
pacman -Scc
```
3. Install some pre-requisites for the build system:
```
pacman -S base-devel git zip
```
4. Install the 32-bit compiler toolchain:
```
pacman -S mingw-w64-i686-autotools mingw-w64-i686-toolchain
```

Certain compilers may be omitted by adding e.g. `--ignore=mingw-w64-i686-gcc-ada,mingw-w64-i686-gcc-fortran,mingw-w64-i686-gcc-libgfortran,mingw-w64-i686-gcc-objc --needed` to the command line.

5. Install all mandatory and optional build dependencies:
```
pacman -S mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_mixer mingw-w64-i686-SDL2_net 
pacman -S mingw-w64-i686-libpng mingw-w64-i686-libsamplerate mingw-w64-i686-python-pillow
```

Additionally, the [pandoc](https://github.com/jgm/pandoc) package may get installed to convert some documentation into HTML format, though this is strictly optional.

6. Next, clone the Crispy Doom source code repository from the GitHub project page:
```
git clone https://github.com/fabiangreffrath/crispy-doom.git
cd crispy-doom
```

If this doesn't work for some odd reason, try to replace the `https` protocol with plain `http`.

7. Bootstrap, configure and compile the sources (this may take some time). The resulting binaries will be in the `src/` directory:
```
autoreconf -fiv
./configure
make
```

If you are feeling adventurous, you may pass the `--enable-truecolor` parameter to the `./configure` call to build the experimental truecolor renderer (Doom only).

8. To keep your sources and binaries updated, run the following in the source directory:
```
git pull
make
```

Usually, Crispy is built with debugging symbols. To strip some bloat off the binaries, call this:
```
strip src/*.exe
```
