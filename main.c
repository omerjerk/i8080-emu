#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "8080emu.h"
#include "shader.h"
#include "main.h"

#define BYTES_PER_PIXEL 3

void readFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset) {
    FILE *f= fopen(filename, "rb");
    if (f==NULL) {
        printf("error: Couldn't open %s\n", filename);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    uint8_t *buffer = &state->memory[offset];
    fread(buffer, fsize, 1, f);
    fclose(f);
}

State8080* init8080(void) {
    State8080* state = calloc(1, sizeof(State8080));
    state->memory = malloc(0x10000);  //16K
    return state;
}

//Returns time in microseconds
double timeusec() {
    //get time
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    //convert from seconds to microseconds
    return ((double)currentTime.tv_sec * 1E6) + ((double)currentTime.tv_usec);
}

void* emulatorThreadFun(void* arg) {
    State8080* state = (State8080*) arg;

    int done = 1;
    double now;
    int whichInt = 1;
    double lastInterrupt = 0.0;
    while (done == 1) {
        done = emulate8080(state);
        now = timeusec();
        if ( now - lastInterrupt > 16667) // 1/60 seconds has elapsed
        {
            // only do an interrupt if they are enabled
            if (state->int_enable) {
                if (whichInt == 1) {
                    whichInt = 2;
                }
                if (whichInt == 2) {
                    whichInt = 1;
                }
                generateInterrupt(state, 2); // Interrupt 2
                // Save the time we did this
                lastInterrupt = now;
            }
        }
    }
}

static void
initGL(GtkWidget* glArea, gpointer data) {
    DisplayStateWrapper* w = data;

    gtk_gl_area_make_current (GTK_GL_AREA (glArea));

	glGenVertexArrays(1, &w->vertexArrayId);
	glBindVertexArray(w->vertexArrayId);

    // Create and compile our GLSL program from the shaders
    w->programId = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");

    GLuint textureId;
    glGenTextures(1, &textureId);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, COLS, ROWS, 0, GL_RGB, GL_UNSIGNED_BYTE, w->img);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    float vertices[] = {
        // positions         // texture coords
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f,   0.0f, 1.0f,   // bottom right
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,   // bottom left
       -1.0f,  1.0f, 0.0f,   1.0f, 0.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &w->vertexBuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, w->vertexBuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &w->elementBufer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->elementBufer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    w->texId = textureId;
}

static gboolean
render (GtkGLArea* area, GdkGLContext* context, gpointer data) {
    // inside this function it's safe to use GL; the given
    // #GdkGLContext has been made current to the drawable
    // surface used by the #GtkGLArea and the viewport has
    // already been set to be the size of the allocation
    DisplayStateWrapper* w = data;
    State8080* state = w->state;

    // we can start by clearing the buffer
    glClearColor (1.0f, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(w->programId);

    // draw your object
    for (int r = 0; r < ROWS; r++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 8; ++k) {
                w->img[(r * COLS + j) + k + (j * 8)] = (state->memory[0x2400 + r * 32 + j] & (1 << k)) == 0? 0 : 255;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, w->texId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, COLS, ROWS, GL_LUMINANCE, GL_UNSIGNED_BYTE, w->img);

    glBindVertexArray(w->vertexArrayId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, w->elementBufer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // we completed our drawing; the draw commands will be
    // flushed at the end of the signal emission chain, and
    // the buffers will be drawn on the window
    return TRUE;
}

gboolean
force_render (gpointer data) {
    gtk_gl_area_queue_render((GtkGLArea*) data);
    return GL_TRUE;
}

static void
start_window (GtkApplication* app, gpointer user_data) {
    GtkWidget* window;
    State8080* state = user_data;

    DisplayStateWrapper* w = (DisplayStateWrapper*) malloc(sizeof(DisplayStateWrapper));
    w->state = state;

    GtkWidget* gl_area = gtk_gl_area_new ();
    gtk_gl_area_set_auto_render((GtkGLArea*) gl_area, GL_FALSE);
    g_signal_connect (gl_area, "realize", G_CALLBACK (initGL), w);
    g_signal_connect (gl_area, "render", G_CALLBACK (render), w);

    g_timeout_add(100, force_render, gl_area);

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 256, 224);
    gtk_container_add(GTK_CONTAINER(window), gl_area);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all (window);
}

int main (int argc, char**argv) {

    State8080* state = init8080();

    readFileIntoMemoryAt(state, "games/space-invaders/invaders.h", 0);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.g", 0x800);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.f", 0x1000);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.e", 0x1800);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, emulatorThreadFun, state); 

    GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (start_window), state);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return 0;
}