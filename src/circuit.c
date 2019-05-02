#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"

/*
struct circuit_s {
    guint nextID; // next free ID for this circuit
    GList* nodeList;
    GList* wireList;
} circuit_t;
*/

circuit_t* createCircuit() {
    circuit_t* c = malloc(sizeof(circuit_t));
    c->nextID = 0;
    c->nodeList = NULL;
    c->wireList = NULL;
    return c;
}

void freeCircuit(circuit_t* c) {
    clearCircuit(c);
    free(c);
}
