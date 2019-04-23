
#include <gtk/gtk.h>
#include <math.h>
#include "vec2.h"
#include "node.h"
#include "wire.h"

int currentID = 0;

GList* nodeList = NULL;

#define nodeWidth 64
#define nodeHeight 64

GdkPixbuf* typeImg[7];
GdkPixbuf* invTypeImg[7];

char typeNames[7][8] = {
    "SPLIT",
    "NOT",
    "AND",
    "OR",
    "XOR",
    "IN",
    "OUT"
};

// pesky XNOR can't just prepend N !!!
char invTypeNames[7][8] = {
    "SPLIT",
    "THRU",
    "NAND",
    "NOR",
    "XNOR",
    "IN",
    "OUT"
};

vec2_t ioPoints[16];

void calcIoPoints()
{
    for (int i = 0; i < 8; i++) {
        ioPoints[i] = (vec2_t) {
            nodeWidth / 2.0 - 4.0,
                      (nodeHeight / 8.0)*(i + 0.5) - (nodeHeight / 2.0)
        };
        ioPoints[i + 8] = (vec2_t) {
            -nodeWidth / 2.0 + 4.0,
                (nodeHeight / 8.0)*(i + 0.5) - (nodeHeight / 2.0)
            };
    }
}

// returns a newly created node also adds it to the node list
node_t* addNode(enum nodeType tp, double x, double y)
{
    node_t* n = malloc(sizeof(node_t));
    n->id = currentID++;
    n->type = tp;
    n->pos.x = x;
    n->pos.y = y;
    n->rotation = 0;
    n->invert = FALSE;
    n->state = FALSE;
    n->text[0] = 0;
    for (int i = 0; i < 8; i++) {
        n->outputs[i].index = i;
        n->outputs[i].wire = NULL;
        n->outputs[i].highlight = FALSE;
        n->inputStates[i] = FALSE;
        n->inputs[i].highlight = FALSE;
        n->inputs[i].index = i;
        n->inputs[i].wire = NULL;
    }

    if (tp == n_and || tp == n_or || tp == n_xor) {
        n->maxInputs = 8;
        n->maxOutputs = 8;
    }
    if (tp == n_in) {
        n->maxInputs = 0;
        n->maxOutputs = 8;
    }
    if (tp == n_not) {
        n->maxInputs = 1;
        n->maxOutputs = 8;
    }
    if (tp == n_split) {
        n->maxInputs = 1;
        n->maxOutputs = 8;
    }
    if (tp == n_out) {
        n->maxInputs = 1;
        n->maxOutputs = 0;
    }

    nodeList = g_list_append(nodeList, n);
    return n;
}

void freeNode(node_t* n)
{
    nodeList = g_list_remove(nodeList, n);
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

// TODO this is horrible and doesn't work properly
// tried many different things.... (give up for now loads more to do!)
// would pango really be any better ?
/*
void vText(cairo_t *cr, const char* str)
{
    cairo_text_extents_t ex;
    cairo_matrix_t save;
    cairo_matrix_t matrix;
    cairo_get_font_matrix(cr, &save);

    // get the height of a char
    char fc[] = { 'X', '\0'};
    cairo_text_extents(cr, fc, &ex);
    double w = ex.width + ex.x_bearing;

    cairo_get_font_matrix(cr, &matrix);

    int l = strlen(str);
    cairo_move_to(cr, -w / 2, -((double)l) * 2.5);
    for (int i = 0; i < l; i++) {
        fc[0] = str[i];
        cairo_show_text (cr, fc);
        cairo_rel_move_to(cr, -w, w * 1.4);
    }
    cairo_set_font_matrix(cr, &save);
}
*/
void drawNode(cairo_t *cr, node_t* n)
{
    cairo_matrix_t before, local;

    cairo_get_matrix(cr, &before);
    cairo_translate(cr, n->pos.x, n->pos.y);
    cairo_rotate(cr, n->rotation);
    cairo_get_matrix(cr, &local);
    
    drawBox(cr, nodeWidth, nodeHeight, n->state);
    cairo_set_matrix(cr, &local);
/*
    if (n->invert) {
        vText(cr, invTypeNames[n->type]);
    } else {
        vText(cr, typeNames[n->type]);
    }
    cairo_stroke (cr);
*/
    cairo_set_line_width(cr, 1);
    for (int i = 0; i < 8; i++) {
        if (i < n->maxOutputs) {
            if (n->outputs[i].highlight) {
                cairo_set_source_rgb(cr, 1, 1, 0);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
            }
            cairo_arc(cr, ioPoints[i].x, ioPoints[i].y, 4, 0, 2 * PI);
            cairo_stroke(cr);
        }

        if (i < n->maxInputs) {
            if (n->inputs[i].highlight) {
                cairo_set_source_rgb(cr, 1, 1, 0);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
            }
            cairo_arc(cr, ioPoints[i + 8].x, ioPoints[i + 8].y, 4, 0, 2 * PI);
            cairo_stroke(cr);
        }
    }
    
    if (n->invert) {    
        gdk_cairo_set_source_pixbuf (cr, invTypeImg[n->type], -24, -24);
    } else {
        gdk_cairo_set_source_pixbuf (cr, typeImg[n->type], -24, -24);
    }
    cairo_paint(cr);
    
    
    cairo_text_extents_t ex;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_text_extents(cr, n->text, &ex);
    cairo_move_to(cr, -ex.width*.5, -nodeHeight*.6);
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

    lx -= n->pos.x - (nodeWidth / 2.0);
    ly -= n->pos.y - (nodeHeight / 2.0);

    return lx >= -n->pos.x && lx <= nodeWidth - n->pos.x &&
           ly >= -n->pos.y && ly <= nodeHeight - n->pos.y;
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
                    fabs(ioPoints[i].y - l.y) < 4.0) {
                return i;
            }
        }

        if (i < n->maxInputs) {
            if (fabs(ioPoints[i + 8].x - l.x) < 4.0 &&
                    fabs(ioPoints[i + 8].y - l.y) < 4.0) {
                return i + 8;
            }
        }
    }
    return -1;
}

void clearCircuit()
{
    while (wireList) {
        wire_t* w = (wire_t*)wireList->data;
        deleteWire(w);
    }

    while (nodeList) {
        node_t* n = (node_t*)nodeList->data;
        freeNode(n);
    }
    currentID = 0;
}

void updateLogic()
{
    GList* it;
    for (it = nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        gboolean states[] = {0,0,0,0,0,0,0,0};
        int stateCount = 0;
        for (int i = 0; i < 8; i++) {
            if (n->inputs[i].wire) {
                states[stateCount] = n->inputStates[i];
                stateCount++;
            }
        }
        
        if (n->type == n_in) {
            continue;
        }

        if (n->type == n_out) {
            n->state = states[0];
            continue;
        }
        
        if (n->type == n_not) {
            if (n->invert) {
                n->state = states[0];
            } else {
                n->state = !states[0];
            }
            continue;
        }
        int newState = states[0];

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
        if (n->invert) {
            newState = !newState;
        }
        n->state = newState;
/*
        if (stateCount == 2) {
            if (n->invert) {
                if (n->type == n_and) {
                    n->state = !(states[0] & states[1]);
                }
                if (n->type == n_or) {
                    n->state = !(states[0] | states[1]);
                }
                if (n->type == n_xor) {
                    n->state = !(states[0] ^ states[1]);
                }
            } else {
                if (n->type == n_and) {
                    n->state = states[0] & states[1];
                }
                if (n->type == n_or) {
                    n->state = states[0] | states[1];
                }
                if (n->type == n_xor) {
                    n->state = states[0] ^ states[1];
                }
            }
        }
        if (stateCount == 3) {
            if (n->invert) {
                if (n->type == n_and) {
                    n->state = !(states[0] & states[1] & states[2]);
                }
                if (n->type == n_or) {
                    n->state = !(states[0] | states[1] | states[2]);
                }
                if (n->type == n_xor) {
                    n->state = !(states[0] ^ states[1] ^ states[2]);
                }
            } else {
                if (n->type == n_and) {
                    n->state = states[0] & states[1] & states[2];
                }
                if (n->type == n_or) {
                    n->state = states[0] | states[1] | states[2];
                }
                if (n->type == n_xor) {
                    n->state = states[0] ^ states[1] ^ states[2];
                }
            }
        }
        if (stateCount == 4) {
            if (n->invert) {
                if (n->type == n_and) {
                    n->state = !(states[0] & states[1] & states[2] & states[3]);
                }
                if (n->type == n_or) {
                    n->state = !(states[0] | states[1] | states[2] | states[3]);
                }
                if (n->type == n_xor) {
                    n->state = !(states[0] ^ states[1] ^ states[2] ^ states[3]);
                }
            } else {
                if (n->type == n_and) {
                    n->state = states[0] & states[1] & states[2] & states[3];
                }
                if (n->type == n_or) {
                    n->state = states[0] | states[1] | states[2] | states[3];
                }
                if (n->type == n_xor) {
                    n->state = states[0] ^ states[1] ^ states[2] ^ states[3];
                }
            }
        }
        if (stateCount == 5) {
            if (n->invert) {
                if (n->type == n_and) {
                    n->state = !(states[0] & states[1] & states[2] & states[3] &
                    states[4]);
                }
                if (n->type == n_or) {
                    n->state = !(states[0] | states[1] | states[2] | states[3] &
                    states[4]);
                }
                if (n->type == n_xor) {
                    n->state = !(states[0] ^ states[1] ^ states[2] ^ states[3]);
                }
            } else {
                if (n->type == n_and) {
                    n->state = states[0] & states[1] & states[2] & states[3];
                }
                if (n->type == n_or) {
                    n->state = states[0] | states[1] | states[2] | states[3];
                }
                if (n->type == n_xor) {
                    n->state = states[0] ^ states[1] ^ states[2] ^ states[3];
                }
            }
        }
    */
        
    }
}
