#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "8080emu.h"

#ifndef MAIN_H
#define MAIN_H

#define COLS 256
#define ROWS 224

typedef struct _DisplayStateWrapper {
    State8080* state;
    GLuint texId;
    GLuint programId;
    GLuint vertexBuffer;
    char img[ROWS*COLS];
} DisplayStateWrapper;

#endif