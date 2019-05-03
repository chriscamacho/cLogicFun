#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"

GtkWidget* graph;
GtkWidget* graphWin;

GList* labels = NULL;
GList* points = NULL;

// will eventually have more controls...
void initGraphWin(GtkBuilder* builder, GtkWidget* mainWin) {
    graph = GTK_WIDGET(gtk_builder_get_object(builder, "graph"));
    graphWin = GTK_WIDGET(gtk_builder_get_object(builder, "graphWin"));
    gtk_window_set_transient_for((GtkWindow*)graphWin, (GtkWindow*)mainWin);
    
}

void showGraph(circuit_t* cir) {
    
    #define mxPoints 160
    
    GList* it;
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_out) {
            //printf("%s ,",n->text);
            labels = g_list_append(labels, &n->text);
            int* p = malloc(sizeof(int)* mxPoints);
            points = g_list_append(points, p);
        }
    }
    
    //printf("\n");
    for (int i=0; i<mxPoints; i++) {
        propagateWires(cir);
        updateLogic(cir);
        GList* pit = points;
        for (it = cir->nodeList; it; it = it->next) {
            node_t* n = (node_t*)it->data;
            if (n->type == n_out) {
                int* ip = (int*)pit->data;
                ip[i] = 1-n->state; // inverted because y+ is down
                //printf("%i, ",n->state);
                pit = pit->next;
            }
            
        }
        //printf("\n");
    }
    gtk_widget_show(graphWin);
}


gboolean onGraphOK(GtkWidget *widget, gpointer data) {
    (void)data;
    (void)widget;
    g_list_free_full(points, free);
    g_list_free(labels);
    labels = NULL;
    points = NULL;
    gtk_widget_hide(graphWin);
    return FALSE;
}

gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    (void)data;
    (void)widget;
    
    cairo_set_line_width(cr, 1);    
    cairo_set_source_rgb(cr, 0.25, 0.25, 0.5);
    for (int x=0; x<640; x=x+4) {
        cairo_move_to(cr, x, 0);        
        cairo_line_to(cr, x, 100);        
    }
    cairo_stroke(cr);
    
    cairo_set_line_width(cr, 2);    
    cairo_set_source_rgb(cr, 0, 0, 0);
    GList* pit;
    GList* lit = labels;
    int yy = 4;
    for (pit = points; pit; pit=pit->next) {
        yy+=12;
        int* pp = (int*)pit->data;
        cairo_move_to(cr, 2, yy);
        cairo_show_text (cr, (char*)lit->data);
        cairo_move_to(cr, 4, pp[0]*4 + yy);
        for (int i=1; i<mxPoints; i++) {
            cairo_line_to(cr, i*4, pp[i]*4 + yy);
            cairo_line_to(cr, 4+i*4, pp[i]*4 + yy);
        }
        cairo_stroke(cr);
        lit=lit->next;
    }


    
    return FALSE;
}
