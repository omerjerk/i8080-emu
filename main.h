#include <gtk/gtk.h>

#include "8080emu.h"

#ifndef MAIN_H
#define MAIN_H

typedef struct _DisplayStateWrapper {
    GtkImage* image;
    State8080* state;
} DisplayStateWrapper;

#endif