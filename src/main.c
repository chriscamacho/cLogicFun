#include <gtk/gtk.h>
#include "vec2.h"
#include "node.h"
#include "wire.h"
#include "xmlLoader.h"
#include "callbacks.h"
#include "nodeWin.h"

#define PREFIX "/uk/co/bedroomcoders/cLogicToy/"

GtkWidget *drawArea;

gboolean timeOut(gpointer data)
{
    (void)data;
    
    propagateWires();
    updateLogic();
    
    gtk_widget_queue_draw(drawArea);

    return TRUE;
}



int main(int argc, char *argv[])
{
    //srand(time(NULL)); // for wire colours

    GtkBuilder      *builder;

    GtkWidget       *window;
    

    calcIoPoints();
    gtk_init(&argc, &argv);

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
