#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "xmlLoader.h"
#include "callbacks.h"
#include "nodeWin.h"
#include "graph.h"
#include "pins.h"

#define PREFIX "/uk/co/bedroomcoders/cLogicFun/"

void initCallbacks(GtkWidget* da, GtkBuilder* builder);

circuit_t* circuit;
circuit_t* currentCircuit;

GdkPixbuf* loadPb(const char* fn)
{
    GError *gerror = NULL;
    GdkPixbuf* pb = gdk_pixbuf_new_from_resource (fn, &gerror);

    if (!pb) {
        printf("error message: %s\n", gerror->message);
        g_error_free(gerror);
    }

    return pb;
}

int main(int argc, char *argv[])
{
    circuit = createCircuit();

    GtkBuilder      *builder;
    GtkWidget       *window;
    GtkWidget       *drawArea;

    calcIoPoints();
    gtk_init(&argc, &argv);

    typeImg[0] = loadPb(PREFIX"res/1.png");
    typeImg[1] = loadPb(PREFIX"res/not.png");
    typeImg[2] = loadPb(PREFIX"res/and.png");
    typeImg[3] = loadPb(PREFIX"res/or.png");
    typeImg[4] = loadPb(PREFIX"res/xor.png");
    typeImg[5] = loadPb(PREFIX"res/input.png");
    typeImg[6] = loadPb(PREFIX"res/output.png");
    typeImg[7] = loadPb(PREFIX"res/src.png");
    typeImg[8] = loadPb(PREFIX"res/dst.png");
    typeImg[9] = loadPb(PREFIX"res/sub.png");

    invTypeImg[0] = loadPb(PREFIX"res/0.png");
    invTypeImg[1] = loadPb(PREFIX"res/buffer.png");
    invTypeImg[2] = loadPb(PREFIX"res/nand.png");
    invTypeImg[3] = loadPb(PREFIX"res/nor.png");
    invTypeImg[4] = loadPb(PREFIX"res/xnor.png");
    invTypeImg[5] = typeImg[5];
    invTypeImg[6] = typeImg[6];
    invTypeImg[7] = typeImg[7];
    invTypeImg[8] = typeImg[8];
    invTypeImg[9] = typeImg[9];


    builder = gtk_builder_new();

    gtk_builder_add_from_resource(builder, PREFIX"res/ui.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    drawArea = GTK_WIDGET(gtk_builder_get_object(builder, "drawArea"));

    initNodeWin(builder, window);
    initGraphWin(builder, window);
    initPinsWin(builder, window);

    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show(window);

    setOffset( gtk_widget_get_allocated_width (drawArea) / 2.0,
               gtk_widget_get_allocated_height (drawArea) / 2.0);

    initCallbacks(drawArea, builder);
    g_object_unref(builder);

    if (argc==2) {
        loadCircuit(circuit, argv[1]);
    }


    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    freeCircuit(circuit); // os can do this, but why not...

    gtk_main_quit();
}
