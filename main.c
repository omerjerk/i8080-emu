#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <pthread.h>

#include "8080emu.h"
#include "main.h"

#define COLS 256
#define ROWS 224
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

static void bw_to_rgb(guchar *rgb, guchar *bw, size_t sz) {
  for (size_t i = 0; i < sz; i++)
    for (size_t j = 0; j < BYTES_PER_PIXEL; j++)
      rgb[i * BYTES_PER_PIXEL + j] = bw[i];
}

gboolean
derp (gpointer data) {

    DisplayStateWrapper* w = data;
    guchar bw[ROWS * COLS] = { 0 };

    for (int r = 0; r < ROWS; r++)
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 8; ++k) {
                bw[(r * COLS + j) + k + (j * 8)] = (w->state->memory[0x2400 + r * 32 + j] & (1 << k)) == 0? 0 : 255;
            }
            // bw[r * COLS + c] = rand() %2 ? 0: 255;
        }

    guchar rbw[ROWS * COLS] = {0};
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < ROWS; ++j) {
            rbw[(i * ROWS) + j] = bw[(j * COLS) + (ROWS - i)];
        }
    }

    guchar rgb[sizeof bw * BYTES_PER_PIXEL];
    bw_to_rgb(rgb, rbw, ROWS * COLS);

    GdkPixbuf *pb = gdk_pixbuf_new_from_data(
        rgb,
        GDK_COLORSPACE_RGB,     // colorspace (must be RGB)
        0,                      // has_alpha (0 for no alpha)
        8,                      // bits-per-sample (must be 8)
        ROWS, COLS,             // cols, rows
        ROWS * BYTES_PER_PIXEL, // rowstride
        NULL, NULL              // destroy_fn, destroy_fn_data
    );

    gtk_image_set_from_pixbuf(w->image, pb);

      /* Return true so the function will be called again; returning false removes
       * this timeout function.
       */
      return TRUE;
}

static void
start_window (GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    State8080* state = user_data;

    DisplayStateWrapper* w = (DisplayStateWrapper*) malloc(sizeof(DisplayStateWrapper));
    w->state = state;

    guchar bw[ROWS * COLS] = { 0 };
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            bw[r * COLS + c] = w->state->memory[0x2400 + ((r * COLS) + c)] == 1 ? 255:0;
        }

    guchar rgb[sizeof bw * BYTES_PER_PIXEL];
    bw_to_rgb(rgb, bw, ROWS * COLS);

    GdkPixbuf *pb = gdk_pixbuf_new_from_data(
        rgb,
        GDK_COLORSPACE_RGB,     // colorspace (must be RGB)
        0,                      // has_alpha (0 for no alpha)
        8,                      // bits-per-sample (must be 8)
        COLS, ROWS,             // cols, rows
        COLS * BYTES_PER_PIXEL, // rowstride
        NULL, NULL              // destroy_fn, destroy_fn_data
    );

    GtkImage *image = (GtkImage*) gtk_image_new_from_pixbuf(pb);
    w->image = image;

    g_timeout_add(100, derp, w);

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 256, 224);
    gtk_container_add(GTK_CONTAINER(window), (GtkWidget*) image);
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