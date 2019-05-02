#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"



circuit_t* createCircuit() {
    circuit_t* c = malloc(sizeof(circuit_t));
    //c->nextID = 0;
    c->nodeList = NULL;
    c->wireList = NULL;
    return c;
}

void freeCircuit(circuit_t* c) {
    clearCircuit(c);
    free(c);
}

guint getNextID(circuit_t* c) {
    gboolean found = TRUE;
    guint nid=0;
    while(found) {
        found = FALSE;
        GList* it;
        for (it = c->nodeList; it; it = it->next) {
            node_t* n = (node_t*)it->data;
            if (nid == n->id) {
                found = TRUE;
            }
        }
        
        for (it = c->wireList; it; it = it->next) {
            wire_t* w = (wire_t*)it->data;
            if (nid == w->id) {
                found = TRUE;
            }
        }
        if (found) {
            nid++;
        }
    }
    printf("new id=%i\n",nid);
    return nid;
}
