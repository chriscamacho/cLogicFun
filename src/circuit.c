#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include <strings.h>



circuit_t* createCircuit() {
    circuit_t* c = malloc(sizeof(circuit_t));
    //c->nextID = 0;
    c->nodeList = NULL;
    c->wireList = NULL;
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
    //cir->nextID = 0;
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
    //printf("new id=%i\n",nid);
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
                    if (strcasecmp(nn->text, n->text) == 0) {
                        n->outputList = g_list_append(n->outputList, nn);
                    }
                }
            }
        }
    }   
}
