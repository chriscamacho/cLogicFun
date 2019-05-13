#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "pins.h"
#include <strings.h>



circuit_t* createCircuit() {
    circuit_t* c = malloc(sizeof(circuit_t));
    c->nodeList = NULL;
    c->wireList = NULL;
    c->pinsIn = NULL;
    c->pinsOut = NULL;
    c->nIns = 0;
    c->nOuts = 0;
    c->txtHash = g_hash_table_new(g_str_hash, g_str_equal);
    return c;
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
    g_hash_table_remove_all(cir->txtHash);

    while(cir->pinsIn) {
        pins_t* p = (pins_t*)cir->pinsIn->data;
        cir->pinsIn = g_list_remove(cir->pinsIn, cir->pinsIn->data);
        freePin(p);
    }

    while(cir->pinsOut) {
        pins_t* p = (pins_t*)cir->pinsOut->data;
        cir->pinsOut = g_list_remove(cir->pinsOut, cir->pinsOut->data);
        freePin(p);
    }

}

void freeCircuit(circuit_t* c) {
    clearCircuit(c);
    g_hash_table_destroy(c->txtHash);
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

    return nid;
}


void updateLogic(circuit_t* cir)
{
    GList* it;
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_dst) {
            continue;
        }

        if (n->type != n_sub) {
            gboolean states[] = {0,0,0,0,0,0,0,0};
            int stateCount = 0;
            for (int i = 0; i < 8; i++) {
                if (n->inputs[i].wire) {
                    states[stateCount] = n->inputs[i].state;
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
        } else {
            // type n_sub
            if (n->circuit!=NULL) {
                propagateWires(n->circuit);
                updateLogic(n->circuit);
            }
        }
    }
}


void findSrcTargets(circuit_t* cir) {
    GList* it;
    for (it = cir->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_src) {
            if(n->outputList){
                //g_list_free(n->srcOutputs);
                // TODO find out why g_list_free caused seg fault
                // in gtk internals
                // but only when called from nodeWindow
                while(n->outputList) {
                    n->outputList = g_list_remove(n->outputList, n->outputList->data);
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
                    if (strcasecmp(nn->p_text, n->p_text) == 0) {
                        n->outputList = g_list_append(n->outputList, nn);
                    }
                }
            }
        }
    }
}
