# Intel 8080 Emulator
An Intel 8080 emulator written in C. I built this mainly to understand the working of a CPU, assembly code and some OpenGL.

## Features:
* Wrote a *mostly complete* interpreter to execute 8080's instruction codes to modify the CPU state.
* Using OpenGL to render the graphics so that rendering is completely offloaded to the GPU.
* Window can be resized in any way, thanks to the OpenGL implementation.
* Used GTK+ as the windowing system (Not really a feature though).

## Dependencies
```
sudo apt-get install libgtk-3-dev freeglut3-dev
```
`libepoxy` is also a dependency but somehow it was already present on my system at least.

## Build
```
make
```

## Run
```
./8080
```
## Screenshots

![alt text](https://raw.githubusercontent.com/omerjerk/hello-8080/master/screenshots/demo.gif)
