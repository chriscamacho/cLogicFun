#include <gtk/gtk.h>
#include "vec2.h"
#include "node.h"
#include "wire.h"
#include "xmlLoader.h"
#include "callbacks.h"
#include "nodeWin.h"

#define PREFIX "/uk/co/bedroomcoders/cLogicFun/"

GtkWidget *drawArea;

gboolean timeOut(gpointer data)
{
    (void)data;

    propagateWires();
    updateLogic();


    gtk_widget_queue_draw(drawArea);

    return TRUE;
}

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
    GtkBuilder      *builder;
    GtkWidget       *window;

    calcIoPoints();
    gtk_init(&argc, &argv);

    typeImg[0] = loadPb(PREFIX"res/1.png");
    typeImg[1] = loadPb(PREFIX"res/not.png");
    typeImg[2] = loadPb(PREFIX"res/and.png");
    typeImg[3] = loadPb(PREFIX"res/or.png");
    typeImg[4] = loadPb(PREFIX"res/xor.png");
    typeImg[5] = loadPb(PREFIX"res/input.png");
    typeImg[6] = loadPb(PREFIX"res/output.png");
    
    invTypeImg[0] = loadPb(PREFIX"res/0.png");
    invTypeImg[1] = loadPb(PREFIX"res/thru.png");
    invTypeImg[2] = loadPb(PREFIX"res/nand.png");
    invTypeImg[3] = loadPb(PREFIX"res/nor.png");
    invTypeImg[4] = loadPb(PREFIX"res/xnor.png");
    invTypeImg[5] = typeImg[5];
    invTypeImg[6] = typeImg[6];
    

    builder = gtk_builder_new();

    gtk_builder_add_from_resource(builder, PREFIX"res/ui.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    drawArea = GTK_WIDGET(gtk_builder_get_object(builder, "drawArea"));
    initNodeWin(builder);
    (void)drawArea;
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);
    gtk_widget_show(window);

    setOffset( gtk_widget_get_allocated_width (drawArea) / 2.0,
               gtk_widget_get_allocated_height (drawArea) / 2.0);

    g_timeout_add (250, timeOut, NULL);
    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}
