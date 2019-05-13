
#include <gtk/gtk.h>
#include <math.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "pins.h"


GdkPixbuf* typeImg[10];
GdkPixbuf* invTypeImg[10];

char typeNames[10][8] = {
    "CONST 1",
    "NOT",
    "AND",
    "OR",
    "XOR",
    "IN",
    "OUT",
    "SRC",
    "DST",
    "SUB"
};

// pesky XNOR can't just prepend N !!!
char invTypeNames[10][8] = {
    "CONST 0",
    "BUFFER",
    "NAND",
    "NOR",
    "XNOR",
    "IN",
    "OUT",
    "SRC",
    "DST",
    "SUB"
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
    n->p_text[0] = 0;
    n->outputList = NULL;
    n->circuit = NULL;
    for (int i = 0; i < 8; i++) {
        n->outputs[i].highlight = FALSE;
        n->inputs[i].state = FALSE;
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

    if (tp== n_sub) {
        n->maxInputs = 0;
        n->maxOutputs = 0;
    }

    cir->nodeList = g_list_append(cir->nodeList, n);
    return n;
}

void freeNode(circuit_t* cir, node_t* n)
{
    cir->nodeList = g_list_remove(cir->nodeList, n);
    if (strlen(n->p_text)!=0) {
        g_hash_table_remove (cir->txtHash, n->p_text);
    }
    if (n->type == n_sub) {
        if (n->circuit!=NULL) {
            clearCircuit(n->circuit);
            freeCircuit(n->circuit);
        }
    }

    free(n);
}

void setNodeText(circuit_t* c, node_t* n, const char* tx)
{
    // only track input and output nodes
    if (n->type == n_in || n->type == n_out) {
        // remove old text from hash
        if (strlen(n->p_text)!=0) {
            g_hash_table_remove(c->txtHash, n->p_text);
        }
    }

    strcpy(n->p_text, tx);

    if (n->type == n_in || n->type == n_out) {
        // add new text to hash
        if (strlen(n->p_text)!=0) {
            g_hash_table_insert(c->txtHash, n->p_text, n);
        }
    }
}

node_t* getNodeFromText(circuit_t* cir, const char* str)
{
    return g_hash_table_lookup(cir->txtHash, str);
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

            if (n->type == n_sub) {
                if (n->circuit != NULL) {
                    cairo_text_extents_t ex;
                    cairo_set_source_rgb(cr, 0, 0, 0);
                    GList* pi = g_list_nth (n->circuit->pinsOut,i);
                    pins_t* p = (pins_t*)pi->data;
                    cairo_text_extents(cr, p->node->p_text, &ex);
                    cairo_move_to(cr, ioPoints[i].x+8, (ex.height/2.0)+ioPoints[i].y-((n->height-64)/2.0));
                    cairo_show_text (cr, p->node->p_text);
                    cairo_stroke(cr);
                }
            }
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

            if (n->type == n_sub) {
                if (n->circuit != NULL) {
                    cairo_text_extents_t ex;
                    cairo_set_source_rgb(cr, 0, 0, 0);
                    GList* pi = g_list_nth (n->circuit->pinsIn,i);
                    pins_t* p = (pins_t*)pi->data;
                    cairo_text_extents(cr, p->node->p_text, &ex);
                    cairo_move_to(cr, ioPoints[i + 8].x-(ex.width)-8, (ex.height/2.0)+ioPoints[i + 8].y-((n->height-64)/2.0));
                    cairo_show_text (cr, p->node->p_text);
                    cairo_stroke(cr);
                }
            }

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
    cairo_text_extents(cr, n->p_text, &ex);
    cairo_move_to(cr, -ex.width*.5, -n->height*.6);
    cairo_show_text (cr, n->p_text);

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



