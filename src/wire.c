#include <gtk/gtk.h>
#include "math.h"
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"


// TODO some consecutives are too close (dark), pick a better list of colours!
double wireColours[43][3] = {
    {0x8c, 0x00, 0x00}, {0xcc, 0xc9, 0x99}, {0x7c, 0x98, 0xa6}, {0xe6, 0xb6, 0xf2},
    {0x33, 0x00, 0x00}, {0x4c, 0x66, 0x1a}, {0x00, 0x1b, 0x33}, {0x40, 0x80, 0x33},
    {0xf2, 0x99, 0x79}, {0x60, 0xbf, 0x60}, {0x30, 0x7c, 0xbf}, {0x80, 0x40, 0x72},
    {0xa6, 0x87, 0x7c}, {0x00, 0xf2, 0x20}, {0x00, 0x2e, 0x73}, {0xff, 0x40, 0xbf},
    {0xa6, 0x5b, 0x29}, {0x73, 0x99, 0x7d}, {0xbf, 0xd9, 0xff}, {0x33, 0x26, 0x2f},
    {0x4c, 0x2a, 0x13}, {0x30, 0x40, 0x36}, {0x00, 0x00, 0xff}, {0x99, 0x00, 0x3d},
    {0xf2, 0x81, 0x00}, {0x00, 0x4d, 0x33}, {0x16, 0x00, 0xa6}, {0xff, 0x40, 0x73},
    {0x33, 0x2b, 0x1a}, {0x39, 0xe6, 0xc3}, {0x09, 0x00, 0x40}, {0xff, 0x00, 0x22},
    {0xb2, 0x8f, 0x00}, {0x79, 0xea, 0xf2}, {0x5a, 0x56, 0x73}, {0x73, 0x39, 0x41},
    {0x73, 0x67, 0x39}, {0x00, 0x8f, 0xb3}, {0x89, 0x6c, 0xd9}, {0xcc, 0x99, 0xa0},
    {0xf2, 0xe2, 0x00}, {0x00, 0x52, 0x66}, {0xc2, 0x00, 0xf2}
};


int nextColour = 0;
// TODO for wires and nodes is it worth using glibs allocator ?

wire_t* addWire(circuit_t* cir)
{
    wire_t* w = malloc(sizeof(wire_t));
    //w->id = cir->nextID++;
    w->id = getNextID(cir);
    int r;
    nextColour += 1;
    if (nextColour > 42) {
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
        if (w->target->type == n_src) {
            propagateSrc(state, w);
        }
    }
    
}
