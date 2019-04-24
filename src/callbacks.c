#include <gtk/gtk.h>
#include "vec2.h"
#include "node.h"
#include "wire.h"
#include <math.h>
#include "xmlLoader.h"
#include "nodeWin.h"

// hmmm a blob of globals...
double zoom = 1.0;
vec2_t ds = { 0.0, 0.0 }; // pan delta
gboolean panning = FALSE;
vec2_t offset = { 0.0, 0.0 };
node_t* panNode = NULL; // if NULL, pan whole scene
gboolean wireDragMode = FALSE;
wire_t dragWire;

// because panNode moved gets reset by redraw
gboolean wasMoved = FALSE;


vec2_t centrePos(GtkWidget* w)
{
    return (vec2_t) {
        (-offset.x + gtk_widget_get_allocated_width(w) / 2.0) / zoom,
        (-offset.y + gtk_widget_get_allocated_height(w) / 2.0) / zoom
    };
}

// node adders, want better userdata provision from glade!
// would rather route all to same call back with index in data...
gboolean addSplit(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_split, v.x, v.y);
    return FALSE;
}

gboolean addOut(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_out, v.x, v.y);
    return FALSE;
}

gboolean addIn(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_in, v.x, v.y);
    return FALSE;
}

gboolean addXor(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_xor, v.x, v.y);
    return FALSE;
}

gboolean addOr(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_or, v.x, v.y);
    return FALSE;
}

gboolean addAnd(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_and, v.x, v.y);
    return FALSE;
}

gboolean addNot(GtkWidget *widget, gpointer data)
{
    (void)widget;
    vec2_t v = centrePos((GtkWidget*)data);
    addNode(n_not, v.x, v.y);
    return FALSE;
}

gboolean on_new_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    clearCircuit();
    gtk_widget_queue_draw((GtkWidget*)data);
    return FALSE;
}

gboolean on_open_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          (GtkWindow*)data,
                                          action,
                                          ("_Cancel"),
                                          GTK_RESPONSE_CANCEL,
                                          ("_Open"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_filename ((GtkFileChooser *)dialog, "./examples/*");
    res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        loadCircuit(filename);
        char title[1024];
        char* f = g_strrstr(filename,"/");
        f++;
        sprintf(title,"cLogicFun - %s", f);
        gtk_window_set_title((GtkWindow*)data, title);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
    return FALSE;
}

gboolean onSave(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                          (GtkWindow*)data,
                                          action,
                                          "_Cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "_Save",
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);


    gtk_file_chooser_set_current_name (chooser,
                                       "./examples/*");

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename (chooser);
        FILE * fp;

        fp = fopen (filename, "w");
        fprintf(fp, "<circuit>\n\n");

        GList* it;
        for (it = nodeList; it; it = it->next) {
            node_t* n = (node_t*)it->data;
            fprintf(fp, "<node nodeID=\"%i\">\n", n->id);
            fprintf(fp, "  <pos x=\"%f\" y=\"%f\" rot=\"%f\"/>\n", n->pos.x, n->pos.y, n->rotation);
            fprintf(fp, "  <logic type=\"%i\" inv=\"%i\" />\n", n->type, n->invert);
            fprintf(fp, "  <io maxIn=\"%i\" maxOut=\"%i\" />\n", n->maxInputs, n->maxOutputs);
            // TODO probably need to escape this string...
            fprintf(fp, "  <label text=\"%s\" />\n", n->text);
            fprintf(fp, "</node>\n\n");
        }
        fprintf(fp, "\n\n");

        for (it = wireList; it; it = it->next) {
            wire_t* w = (wire_t*)it->data;
            fprintf(fp, "<wire wireID=\"%i\">\n", w->id);
            fprintf(fp, "  <parent pid=\"%i\" pindex=\"%i\" />\n", w->parent->id, w->outIndex);
            fprintf(fp, "  <target tid=\"%i\" tindex=\"%i\" />\n", w->target->id, w->inIndex);
            fprintf(fp, "  <colour r=\"%f\" g=\"%f\" b=\"%f\" />\n", w->colourR, w->colourG, w->colourB);
            fprintf(fp, "</wire>\n\n");
        }

        fprintf(fp, "\n</circuit>\n");
        fclose(fp);

        g_free (filename);
    }
    gtk_widget_destroy (dialog);
    return FALSE;
}

void setOffset(double x, double y)
{
    offset.x = x;
    offset.y = y;
}

gboolean eventBox_scroll_event_cb(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
    (void)data;

    double oldZoom = zoom;
    zoom -= (((GdkEventScroll*)event)->delta_y / 10.0);

    if (zoom < 0.1) {
        zoom = 0.1;
    }

    if (zoom > 6) {
        zoom = 6;
    }

    if (zoom != oldZoom) {
        // TODO needs more work, attempting to zoom at the mouse pointer
        /*
        double zd = (zoom - oldZoom);

        double hw =gtk_widget_get_allocated_width(widget) / 2.0;
        double hh =gtk_widget_get_allocated_height(widget) / 2.0;

        offset.x += (hw-((GdkEventScroll*)event)->x+offset.x*zoom) * zd;
        offset.y += (hh-((GdkEventScroll*)event)->y+offset.y*zoom) * zd;
        */
        gtk_widget_queue_draw(widget);
    }
    //printf("zoom %f\n", zoom);
    return TRUE;
}



gboolean eventBox_button_release_event_cb( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
    (void)data;
    if(event->button == GDK_BUTTON_PRIMARY) {
        if (panNode && !wasMoved) {
            showNodeWindow(panNode);
        }
    }

    if (event->button == GDK_BUTTON_SECONDARY) {
        if (panNode) {
            if (panNode->type == n_in) {
                panNode->state = !panNode->state;
            }
        }
    }

    if (wireDragMode) {
        if (dragWire.target != NULL) {
            // if new wire is being dragged, and we are over an input
            // delete any wire on the input

            if ((wire_t*)dragWire.target->inputs[dragWire.inIndex].wire) {
                wire_t* w = (wire_t*)dragWire.target->inputs[dragWire.inIndex].wire;
                deleteWire(w);
            }

            // add the new wire from the drag wire info
            wire_t* w = addWire();
            w->parent = dragWire.parent;
            w->target = dragWire.target;
            w->outIndex = dragWire.outIndex;
            w->inIndex = dragWire.inIndex;
            //w->parent->outputs[w->outIndex].wire = w;
            w->target->inputs[w->inIndex].wire = w;
        }
    }
    // finished dragging so reset stuff
    ds = (vec2_t) {
        0.0, 0.0
    };
    panning = FALSE;
    wireDragMode = FALSE;
    dragWire.target = NULL;
    dragWire.parent = NULL;
    dragWire.inIndex = -1;
    dragWire.outIndex = -1;
    wasMoved = FALSE;

    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean eventBox_button_press_event_cb(GtkWidget *widget, GdkEventButton *event)
{
    (void)widget;
    ds = (vec2_t) {
        event->x, event->y
    };
    panNode = NULL;
    dragWire.target = NULL;
    dragWire.parent = NULL;
    GList* it;
    for (it = nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (pointInNode((event->x - offset.x) / zoom,
                        (event->y - offset.y) / zoom, n)) {
            // if dragging in a node with highlighted output
            // delete the existing wire and start dragging the wire
            int hi = -1;
            for (int i = 0; i<8; i++) {
                if (n->outputs[i].highlight) {
                    hi = i;
                    break;
                }  
            }


            if (hi != -1) {
                wireDragMode = TRUE;
                dragWire.cp2 = (vec2_t) {
                    (event->x - offset.x) / zoom,
                    (event->y - offset.y) / zoom
                };

                /*
                int i = -1;
                if (n->outputs[0].highlight) {
                    i = 0;
                }
                if (n->outputs[1].highlight) {
                    i = 1;
                }
                if (n->outputs[2].highlight) {
                    i = 2;
                }
                if (n->outputs[3].highlight) {
                    i = 3;
                }*/
                
                //if (i != -1) { 
                    //if ((wire_t*)n->outputs[hi].wire) {
                    //    wire_t* w = (wire_t*)n->outputs[hi].wire;
                    //    deleteWire(w);
                    //}
                //}
            }
            if (hi==-1) {
                for (int i=0; i<8;i++)
                if (n->inputs[i].highlight) {
                    wire_t* w=(wire_t*)n->inputs[i].wire;
                    if (w) {
                        deleteWire((wire_t*)w);
                    }
                    wireDragMode = FALSE;
                    panNode = NULL;
                    wasMoved = TRUE;
                    break;
                }
            }
            if (!wireDragMode) {
                panNode = n;
                break;
            }
            


        }
    }

    // if not dragging then do panning
    if (!wireDragMode) {
        panning = TRUE;
    }

    return TRUE;
}

gboolean eventBox_motion_notify_event_cb( GtkWidget *widget, GdkEventMotion *event )
{
    (void)widget;

    if(panning && event->state == GDK_BUTTON1_MASK) {
        // change view offset when panning
        double x = (event->x - ds.x); // /zoom;
        double y = (event->y - ds.y); // /zoom;
        if (panNode == NULL) {
            offset.x += x;
            offset.y += y;
        } else {
            panNode->pos.x += x / zoom;
            panNode->pos.y += y / zoom;
            wasMoved = TRUE;
        }
        ds = (vec2_t) {
            event->x, event->y
        };
        gtk_widget_queue_draw(widget);
    }

    double x = (event->x - offset.x) / zoom;
    double y = (event->y - offset.y) / zoom;
    GList* it;
    gboolean redraw = FALSE;
    for (it = nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        gboolean oldv[16];
        // find the state of all the node in/outputs
        for (int i = 0; i < 16; i++) {
            if (i < 8) {
                oldv[i] = n->outputs[i].highlight;
                n->outputs[i].highlight = FALSE;
            } else {
                oldv[i] = n->inputs[i - 8].highlight;
                n->inputs[i - 8].highlight = FALSE;
            }
        }
        // if inside this node
        if (pointInNode(x, y, n)) {
            // if inside a node check output to
            // see if the mouse is over it
            int ni = pointInIo(x, y, n);
            if (ni != -1 && ni < 8 && !wireDragMode) {
                // if its an output reposition dragwire start point
                n->outputs[ni].highlight = TRUE;
                dragWire.outIndex = ni;
                dragWire.parent = n;

                // TODO work out what happening here!
                double ac = cos(-dragWire.parent->rotation + 90.0 * D2R);
                double as = sin(-dragWire.parent->rotation + 90.0 * D2R);
                vec2_t p = {as * ioPoints[ni].x - ac * ioPoints[ni].y,
                            ac * ioPoints[ni].x + as * ioPoints[ni].y
                           };

                p.x += dragWire.parent->pos.x;
                p.y += dragWire.parent->pos.y;

                dragWire.sp = p;

                p.x += (as * (ioPoints[ni].x + 40.0) - ac * ioPoints[ni].y);
                p.y += (ac * (ioPoints[ni].x + 40.0) + as * ioPoints[ni].y);
                dragWire.cp1 = p;
            }

            if (ni > 7) {
                // if its an input set the end of the dragwire
                n->inputs[ni - 8].highlight = TRUE;
                if (wireDragMode) {
                    dragWire.target = n;
                    dragWire.inIndex = ni - 8;
                    dragWire.cp2 = (vec2_t) {
                        (event->x - offset.x) / zoom,
                        (event->y - offset.y) / zoom
                    };
                    double ac = cos(-dragWire.target->rotation + 90.0 * D2R);
                    double as = sin(-dragWire.target->rotation + 90.0 * D2R);
                    dragWire.cp2.x += (as * (ioPoints[ni - 8].x - 40.0) - ac * ioPoints[ni - 8].y);
                    dragWire.cp2.y += (ac * (ioPoints[ni - 8].x - 40.0) + as * ioPoints[ni - 8].y);
                }
            } else {
                // move the end control point if dragging
                if (wireDragMode) {
                    dragWire.cp2 = dragWire.ep;
                }
            }
        }
        // check for changes redraw only if needed
        // (mouse leaving output in inferred)
        for (int i = 0; i < 16; i++) {
            if (i < 8 && oldv[i] != n->outputs[i].highlight) {
                dragWire.parent = n;
                dragWire.outIndex = i;
                redraw = TRUE;
            } else if (i > 7 && oldv[i] != n->inputs[i-8].highlight) {
                redraw = TRUE;
            }
        }

    }
    // while dragging move the dragwire endpoint
    if (wireDragMode) {
        dragWire.ep = (vec2_t) {
            (event->x - offset.x) / zoom,
            (event->y - offset.y) / zoom
        };
        redraw = TRUE;
    }


    if (redraw) {
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}


gboolean drawArea_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    (void)data;
    (void)widget;
    cairo_matrix_t matrix;
    /*
        guint width, height;

        width = gtk_widget_get_allocated_width (widget);
        height = gtk_widget_get_allocated_height (widget);
    */

    cairo_select_font_face (cr, "Mono", CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    cairo_translate(cr, offset.x, offset.y);
    cairo_scale(cr, zoom, zoom);

    cairo_get_matrix (cr, &matrix);

    // centre point mark
    cairo_set_line_width(cr, 1.0 / zoom);
    cairo_move_to(cr, 0, 0);
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    cairo_move_to(cr, -400, 0);
    cairo_line_to(cr, 400, 0);
    cairo_move_to(cr, 0, -400);
    cairo_line_to(cr, 0, 400);
    cairo_move_to(cr, 0, 0);
    cairo_stroke(cr);

    GList* it;
    for (it = nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        drawNode(cr, n);
    }

    drawWires(cr, zoom);

    if (wireDragMode) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 4.0 / zoom);
        cairo_move_to(cr, dragWire.sp.x, dragWire.sp.y);
        cairo_curve_to(cr, dragWire.cp1.x, dragWire.cp1.y,
                       dragWire.cp2.x, dragWire.cp2.y,
                       dragWire.ep.x, dragWire.ep.y);
        cairo_stroke(cr);
    }

    return FALSE;

}
