# Intel 8080 Emulator
An Intel 8080 emulator written in C. I have used GTK+ as the windowing system and OpenGL to render the graphics.

## Features:
* Wrote a *mostly complete* interpreter to execute 8080's instruction codes to modify the CPU state.
* Using OpenGL to render the graphics so that rendering is completely offloaded to the GPU.
* Written in pure C.

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
This is how the display looks as of now:

![alt text](https://raw.githubusercontent.com/omerjerk/hello-8080/master/screenshots/demo.gif)
