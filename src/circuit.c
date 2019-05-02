#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"



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
