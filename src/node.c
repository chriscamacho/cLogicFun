
#include <gtk/gtk.h>
#include <math.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include <strings.h>




GdkPixbuf* typeImg[9];
GdkPixbuf* invTypeImg[9];

char typeNames[9][8] = {
    "CONST 1",
    "NOT",
    "AND",
    "OR",
    "XOR",
    "IN",
    "OUT",
    "SRC",
    "DST"
};

// pesky XNOR can't just prepend N !!!
char invTypeNames[9][8] = {
    "CONST 0",
    "THRU",
    "NAND",
    "NOR",
    "XNOR",
    "IN",
    "OUT",
    "SRC",
    "DST"
};

/* TODO this needs looking at as width/height not fixed any more */
vec2_t ioPoints[16];

void calcIoPoints()
{
    for (int i = 0; i < 8; i++) {
        ioPoints[i] = (vec2_t) {
            64 / 2.0 - 4.0,
                      (64 / 8.0)*(i + 0.5) - (64 / 2.0)
        };
        ioPoints[i + 8] = (vec2_t) {
            -64 / 2.0 + 4.0,
                (64 / 8.0)*(i + 0.5) - (64 / 2.0)
            };
    }
}

// returns a newly created node also adds it to the node list
node_t* addNode(circuit_t* cir, enum nodeType tp, double x, double y)
{
    node_t* n = malloc(sizeof(node_t));
    n->id = getNextID(cir);//cir->nextID++;
    n->type = tp;
    n->pos.x = x;
    n->pos.y = y;
    n->rotation = 0;
    n->width = 64;
    n->height = 64;
    n->invert = FALSE;
    n->state = FALSE;
    n->text[0] = 0;
    n->outputWires = NULL;
    n->srcOutputs = NULL;
    for (int i = 0; i < 8; i++) {
        n->outputs[i].highlight = FALSE;
        n->inputStates[i] = FALSE;
        n->inputs[i].highlight = FALSE;
        n->inputs[i].wire = NULL;
        
        n->stateBuffer[i] = 0;
    }
    
    n->latency = 0;

    if (tp == n_and || tp == n_or || tp == n_xor) {
        n->maxInputs = 8;
        n->maxOutputs = 1;
    }
    if (tp == n_in || tp == n_const || tp == n_dst) {
        n->maxInputs = 0;
        n->maxOutputs = 1;
    }
    if (tp == n_not) {
        n->maxInputs = 1;
        n->maxOutputs = 1;
    }

    if (tp == n_out || tp == n_src) {
        n->maxInputs = 1;
        n->maxOutputs = 0;
    }
    
    if (tp == n_src || tp == n_dst) {
        n->height = 24;
    }

    cir->nodeList = g_list_append(cir->nodeList, n);
    return n;
}

void freeNode(circuit_t* cir, node_t* n)
{
    cir->nodeList = g_list_remove(cir->nodeList, n);
    free(n);
}

// adapted from https://www.cairographics.org/samples/rounded_rectangle/
// presumably as no licence its public domain...
void drawBox(cairo_t *cr, double width, double height, gboolean active)
{
#define radius 4
    vec2_t d = { width / 2.0, height / 2.0 };
    cairo_new_sub_path (cr);
    cairo_arc (cr, d.x - radius, radius - d.y, radius, -90 * D2R, 0 * D2R);
    cairo_arc (cr, d.x - radius, d.y - radius, radius, 0 * D2R, 90 * D2R);
    cairo_arc (cr, radius - d.x, d.y - radius, radius, 90 * D2R, 180 * D2R);
    cairo_arc (cr, radius - d.x, radius - d.y, radius, 180 * D2R, 270 * D2R);
    cairo_close_path (cr);

    if (active) {
        cairo_set_source_rgb (cr, 0.5, 1, 1);
    } else {
        cairo_set_source_rgb (cr, 0.25, 0.75, 0.75);
    }
    cairo_fill_preserve (cr);
    cairo_set_source_rgba (cr, 0.5, 0, 0, 0.5);
    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);

}


void drawNode(cairo_t *cr, node_t* n)
{
    cairo_matrix_t before, local;

    cairo_get_matrix(cr, &before);
    cairo_translate(cr, n->pos.x, n->pos.y);
    cairo_rotate(cr, n->rotation);
    cairo_get_matrix(cr, &local);
    
    drawBox(cr, n->width, n->height, n->state);
    cairo_set_matrix(cr, &local);

    for (int i = 0; i < 8; i++) {
        if (i < n->maxOutputs) {
            if (n->outputs[i].highlight) {
                cairo_set_source_rgb(cr, 1, 1, 0);
                cairo_set_line_width(cr, 6);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
                cairo_set_line_width(cr, 2);
            }
            cairo_arc(cr, ioPoints[i].x, ioPoints[i].y-((n->height-64)/2.0), 4, 0, 2 * PI);
            cairo_stroke(cr);
        }

        if (i < n->maxInputs) {
            if (n->inputs[i].highlight) {
                cairo_set_source_rgb(cr, 1, 1, 0);
                cairo_set_line_width(cr, 6);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
                cairo_set_line_width(cr, 2);
            }
            cairo_arc(cr, ioPoints[i + 8].x, ioPoints[i + 8].y-((n->height-64)/2.0), 4, 0, 2 * PI);
            cairo_stroke(cr);
        }
    }

    cairo_set_line_width(cr, 1);
    
    if (n->invert) {
        gdk_cairo_set_source_pixbuf (cr, invTypeImg[n->type], -24, -24);   
    } else {
        if (n->type == n_src || n->type == n_dst) {
            gdk_cairo_set_source_pixbuf (cr, typeImg[n->type], -24, -12);
        } else {
            gdk_cairo_set_source_pixbuf (cr, typeImg[n->type], -24, -24);
        }
    }
    cairo_paint(cr);
    
    
    cairo_text_extents_t ex;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_text_extents(cr, n->text, &ex);
    cairo_move_to(cr, -ex.width*.5, -n->height*.6);
    cairo_show_text (cr, n->text);
    
    cairo_set_matrix(cr, &before);
}

// rotate the point in the boxes local space accounting
// for the nodes centred origin
gboolean pointInNode(double x, double y, node_t* n)
{
    vec2_t r = {n->pos.x - x, n->pos.y - y};
    double ac = cos(-n->rotation);
    double as = sin(-n->rotation);
    double lx = ac * r.x - as * r.y;
    double ly = as * r.x + ac * r.y;

    lx -= n->pos.x - (n->width / 2.0);
    ly -= n->pos.y - (n->height / 2.0);

    return lx >= -n->pos.x && lx <= n->width - n->pos.x &&
           ly >= -n->pos.y && ly <= n->height - n->pos.y;
}

int pointInIo(double x, double y, node_t* n)
{
    vec2_t r = { x - n->pos.x, y - n->pos.y };
    double ac = cos(-n->rotation);
    double as = sin(-n->rotation);
    vec2_t l = { ac * r.x - as * r.y, as * r.x + ac * r.y };

    for (int i = 0; i < 8; i++) {
        if (i < n->maxOutputs) {
            if (fabs(ioPoints[i].x - l.x) < 4.0 &&
                    fabs(ioPoints[i].y-((n->height-64)/2.0) - l.y) < 4.0) {
                return i;
            }
        }

        if (i < n->maxInputs) {
            if (fabs(ioPoints[i + 8].x - l.x) < 4.0 &&
                    fabs(ioPoints[i + 8].y-((n->height-64)/2.0) - l.y) < 4.0) {
                return i + 8;
            }
        }
    }
    return -1;
}

void clearCircuit(circuit_t* cir)
{
    while (cir->wireList) {
        wire_t* w = (wire_t*)cir->wireList->data;
        deleteWire(cir, w);
    }

    while (cir->nodeList) {
        node_t* n = (node_t*)cir->nodeList->data;
        freeNode(cir, n);
    }
    //cir->nextID = 0;
}

void updateLogic(circuit_t* cir)
{
    GList* it;
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_dst) {
            continue;
        }
        gboolean states[] = {0,0,0,0,0,0,0,0};
        int stateCount = 0;
        for (int i = 0; i < 8; i++) {
            if (n->inputs[i].wire) {
                states[stateCount] = n->inputStates[i];
                stateCount++;
            }
        }
        
        int newState = states[0];
        
        if (n->type == n_in) {
            newState = n->state; 
        }
        
        if (n->type == n_not) {
            newState = !states[0];
        }
        
        if (n->type == n_const) {
            newState = 1;
        }

        if (n->type == n_and || n->type == n_or || n->type == n_xor) {

            for (int i=1; i<stateCount; i++) {
                if (n->type == n_and) {
                    newState = newState & states[i];
                }
                if (n->type == n_or) {
                    newState = newState | states[i];
                }
                if (n->type == n_xor) {
                    newState = newState ^ states[i];
                }
            }
            
        }
        
        if (n->invert) {
            newState = !newState;
        }
        n->stateBuffer[7] = n->stateBuffer[6];
        n->stateBuffer[6] = n->stateBuffer[5];
        n->stateBuffer[5] = n->stateBuffer[4];
        n->stateBuffer[4] = n->stateBuffer[3];
        n->stateBuffer[3] = n->stateBuffer[2];
        n->stateBuffer[2] = n->stateBuffer[1];
        n->stateBuffer[1] = n->stateBuffer[0];
        
        n->stateBuffer[0] = newState;
        n->state = n->stateBuffer[n->latency];

    }
}

void findSrcTargets(circuit_t* cir) {
    GList* it;
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_src) {
            if(n->srcOutputs){
                //g_list_free(n->srcOutputs);
                // TODO find out why g_list_free caused seg fault
                // in gtk internals
                // but only when called from nodeWindow
                while(n->srcOutputs) {
                    n->srcOutputs = g_list_remove(n->srcOutputs, n->srcOutputs->data);
                }
            }
        }
    }
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_src) {
            GList* iit;
            for (iit = cir->nodeList; iit; iit=iit->next) {
                node_t* nn = (node_t*)iit->data;
                if (nn->type == n_dst) {
                    if (strcasecmp(nn->text, n->text) == 0) {
                        n->srcOutputs = g_list_append(n->srcOutputs, nn);
                    }
                }
            }
        }
    }   
}
