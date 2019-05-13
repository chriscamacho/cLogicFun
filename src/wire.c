#include <gtk/gtk.h>
#include "math.h"
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "pins.h"


// TODO some consecutives are too close (dark), pick a better list of colours!

double wireColours[][3] = {
    {0x00, 0x00, 0x00},
    {0xff, 0x00, 0x00}, {0x00, 0x00, 0xff}, {0x00, 0xff, 0x00},
    {0xff, 0x00, 0xff}, {0x00, 0xff, 0xff}, {0xff, 0xff, 0x00},
    {0xb3, 0x57, 0x56}, {0x58, 0x56, 0xb3}, {0x56, 0xb3, 0x58},
    {0xb3, 0x00, 0xb0}, {0x00, 0xb0, 0xb3}, {0xb0, 0xb3, 0x00},
};

int nextColour = 0;
// TODO for wires and nodes is it worth using glibs allocator ?

wire_t* addWire(circuit_t* cir)
{
    wire_t* w = malloc(sizeof(wire_t));
    w->id = getNextID(cir);
    int r;
    nextColour += 1;
    if (nextColour > 12) {
        nextColour = 0;
    }
    r = nextColour;

    w->colourR = wireColours[r][0] / 256.0;
    w->colourG = wireColours[r][1] / 256.0;
    w->colourB = wireColours[r][2] / 256.0;
    w->parent = NULL;
    w->target = NULL;
    cir->wireList = g_list_append(cir->wireList, w);
    return w;
}

void updateWire(wire_t* w)
{
    double ac = cos(-w->parent->rotation + 90.0 * D2R);
    double as = sin(-w->parent->rotation + 90.0 * D2R);
    w->sp = (vec2_t) {
        as * ioPoints[w->outIndex].x - ac * (ioPoints[w->outIndex].y-((w->parent->height-64)/2.0)),
        ac * ioPoints[w->outIndex].x + as * (ioPoints[w->outIndex].y-((w->parent->height-64)/2.0))
    };
    w->sp.x += w->parent->pos.x;
    w->sp.y += w->parent->pos.y;

    w->cp1.x = (as * (ioPoints[w->outIndex].x + 60.0 - (w->outIndex * 5.0)) - ac * (ioPoints[w->outIndex].y-((w->parent->height-64)/2.0)));
    w->cp1.y = (ac * (ioPoints[w->outIndex].x + 60.0 - (w->outIndex * 5.0)) + as * (ioPoints[w->outIndex].y-((w->parent->height-64)/2.0)));
    w->cp1.x += w->parent->pos.x;
    w->cp1.y += w->parent->pos.y;

    ac = cos(-w->target->rotation + 90.0 * D2R);
    as = sin(-w->target->rotation + 90.0 * D2R);
    w->ep = (vec2_t) {
        as * ioPoints[w->inIndex + 8].x - ac * (ioPoints[w->inIndex + 8].y-((w->target->height-64)/2.0)),
        ac * ioPoints[w->inIndex + 8].x + as * (ioPoints[w->inIndex + 8].y-((w->target->height-64)/2.0))
    };
    w->ep.x += w->target->pos.x;
    w->ep.y += w->target->pos.y;

    w->cp2.x = (as * (ioPoints[w->inIndex + 8].x - 60.0 + (w->inIndex * 5.0)) - ac * (ioPoints[w->inIndex + 8].y-((w->target->height-64)/2.0)));
    w->cp2.y = (ac * (ioPoints[w->inIndex + 8].x - 60.0 + (w->inIndex * 5.0)) + as * (ioPoints[w->inIndex + 8].y-((w->target->height-64)/2.0)));
    w->cp2.x += w->target->pos.x;
    w->cp2.y += w->target->pos.y;
}

void drawWires(cairo_t* cr, circuit_t* cir, double zoom)
{
    GList* it;
    (void)zoom;

    for (it = cir->wireList; it; it = it->next) {
        wire_t* w = (wire_t*)it->data;
        if (w->state) {
            cairo_set_line_width(cr, 6.0);
        } else {
            cairo_set_line_width(cr, 3.0);
        }
        updateWire(w);
        cairo_set_source_rgb(cr, w->colourR, w->colourB, w->colourG);
        cairo_move_to(cr, w->sp.x, w->sp.y);
        cairo_curve_to(cr, w->cp1.x, w->cp1.y, w->cp2.x, w->cp2.y, w->ep.x, w->ep.y);
        cairo_stroke(cr);
    }
}


void deleteWire(circuit_t* cir, wire_t* w)
{
    cir->wireList = g_list_remove (cir->wireList, w);
    w->parent->outputList = g_list_remove(w->parent->outputList, w);
    w->target->inputs[w->inIndex].wire = NULL;
    free(w);
}

void propagateSrc(gboolean state, wire_t* w) {
    GList* it,*iit;

    for (it = w->target->outputList; it; it=it->next) {
        node_t* n = (node_t*)it->data;
        n->state = state;
        for (iit=n->outputList; iit; iit=iit->next) {
            wire_t* ww = (wire_t*)iit->data;
            ww->target->inputs[ww->inIndex].state = state;
            ww->state = state;
            if (ww->target->type == n_src) {
                propagateSrc(state,ww->target->inputs[0].wire);
            }
        }
    }
}

void propagateWires(circuit_t* cir)
{
    GList* it;

    for (it = cir->wireList; it; it = it->next) {
        wire_t* w = (wire_t*)it->data;
        gboolean state = w->parent->state;
        w->target->inputs[w->inIndex].state = state;
        w->state=state;
        if (w->parent->type == n_sub && w->parent->circuit != NULL) {
            GList* pi = g_list_nth (w->parent->circuit->pinsOut,w->outIndex);
            pins_t* p = (pins_t*)pi->data;
            w->state = p->node->state;
            w->target->inputs[w->inIndex].state = p->node->state;
        }
        if (w->target->type == n_src) {
            propagateSrc(state, w);
        }
        if (w->target->type == n_sub && w->target->circuit != NULL) {
            GList* pi = g_list_nth(w->target->circuit->pinsIn,w->inIndex);
            pins_t* p = (pins_t*)pi->data;
            p->node->state = w->state;
        }
    }

}
