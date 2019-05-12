#include <gtk/gtk.h>

#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "pins.h"
#include "xmlLoader.h"

#include <expat.h>
#include <stdio.h>
#include <strings.h>


#define BUFFSIZE        8192
char Buff[BUFFSIZE];

node_t newNode;
wire_t newWire;
pins_t pinIn,pinOut;

int pid, tid;

GHashTable* hash;

circuit_t* currentCircuit;


// as each xml tag is first encountered the callback is triggered
static void XMLCALL start(void *data, const XML_Char *el, const XML_Char **attr)
{
    int i;
    (void)data;
    (void)el;


    for (i = 0; attr[i]; i += 2) {

        if (strcasecmp("pinIn", el) == 0) {
            if (strcasecmp("destID", attr[i]) == 0) {
                pinIn.tmpID = atoi(attr[i + 1]);
            }
            if (strcasecmp("pin", attr[i]) == 0) {
                pinIn.pin = atoi(attr[i + 1]);
            }
        }

        if (strcasecmp("pinOut", el) == 0) {
            if (strcasecmp("destID", attr[i]) == 0) {
                pinOut.tmpID = atoi(attr[i + 1]);
            }
            if (strcasecmp("pin", attr[i]) == 0) {
                pinOut.pin = atoi(attr[i + 1]);
            }
        }


        if (strcasecmp("node", el) == 0) {
            if (strcasecmp("nodeID", attr[i]) == 0) {
                newNode.id = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("pos", el) == 0) {
            if (strcasecmp("x", attr[i]) == 0) {
                newNode.pos.x = atoi(attr[i + 1]);
            }
            if (strcasecmp("y", attr[i]) == 0) {
                newNode.pos.y = atoi(attr[i + 1]);
            }
            if (strcasecmp("rot", attr[i]) == 0) {
                newNode.rotation = atof(attr[i + 1]);
            }
        }
        if (strcasecmp("logic", el) == 0) {
            if (strcasecmp("type", attr[i]) == 0) {
                newNode.type = atoi(attr[i + 1]);
            }
            if (strcasecmp("inv", attr[i]) == 0) {
                newNode.invert = atoi(attr[i + 1]);
            }
            if (strcasecmp("latency", attr[i]) == 0) {
                newNode.latency = atoi(attr[i + 1]);
            }
            if (strcasecmp("state", attr[i]) == 0) {
                newNode.state = atoi(attr[i + 1]);
            }

        }
        if (strcasecmp("io", el) == 0) {
            if (strcasecmp("maxIn", attr[i]) == 0) {
                newNode.maxInputs = atoi(attr[i + 1]);
            }
            if (strcasecmp("maxOut", attr[i]) == 0) {
                newNode.maxOutputs = atoi(attr[i + 1]);
            }
        }

        if (strcasecmp("wire", el) == 0) {
            if (strcasecmp("wireID", attr[i]) == 0) {
                newWire.id = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("parent", el) == 0) {
            if (strcasecmp("pid", attr[i]) == 0) {
                pid = atoi(attr[i + 1]);
            }
            if (strcasecmp("pindex", attr[i]) == 0) {
                newWire.outIndex = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("target", el) == 0) {
            if (strcasecmp("tid", attr[i]) == 0) {
                tid = atoi(attr[i + 1]);
            }
            if (strcasecmp("tindex", attr[i]) == 0) {
                newWire.inIndex = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("colour", el) == 0) {
            if (strcasecmp("r", attr[i]) == 0) {
                newWire.colourR = atof(attr[i + 1]);
            }
            if (strcasecmp("g", attr[i]) == 0) {
                newWire.colourG = atof(attr[i + 1]);
            }
            if (strcasecmp("b", attr[i]) == 0) {
                newWire.colourB = atof(attr[i + 1]);
            }
        }
        if (strcasecmp("label", el) == 0) {
            if (strcasecmp("text", attr[i]) == 0) {
                // newNode never directly added to circuit so safe to directly modify
                strcpy(newNode.p_text, attr[i + 1]);
            }
        }
    }

}

// when the end of the xml tag is encountered this is called
// the temporary node or wire is transfered to the circuit,
static void XMLCALL
end(void *data, const XML_Char *el)
{
    (void)data;
    (void)el;

    if (strcasecmp("node", el) == 0) {
        // add node from newNode
        node_t* n = addNode(currentCircuit, newNode.type, newNode.pos.x, newNode.pos.y);
        n->id = newNode.id;
        g_hash_table_insert(hash, GINT_TO_POINTER(n->id), n);
        n->rotation = newNode.rotation;
        n->invert = newNode.invert;
        n->maxInputs = newNode.maxInputs;
        n->maxOutputs = newNode.maxOutputs;
        n->latency = newNode.latency;
        n->state = newNode.state;
        for (int i=0; i<8; i++) {
            n->stateBuffer[i] = n->state;
        }
        setNodeText(currentCircuit, n, &newNode.p_text[0]);
    }

    if (strcasecmp("wire", el) == 0) {
        // add wire from newWire
        wire_t* w = addWire(currentCircuit);
        w->id = newWire.id;
        w->parent = g_hash_table_lookup(hash, GINT_TO_POINTER(pid));
        w->target = g_hash_table_lookup(hash, GINT_TO_POINTER(tid));
        w->inIndex = newWire.inIndex;
        w->outIndex = newWire.outIndex;
        w->colourR = newWire.colourR;
        w->colourG = newWire.colourG;
        w->colourB = newWire.colourB;

        w->parent->outputList = g_list_append(w->parent->outputList, w);
        w->target->inputs[w->inIndex].wire = w;
    }

    if (strcasecmp("pinIn", el) == 0) {
        pins_t* p = createPin(NULL);
        p->tmpID = pinIn.tmpID;
        p->pin = pinIn.pin;
        currentCircuit->pinsIn = g_list_append(currentCircuit->pinsIn, p);
    }

    if (strcasecmp("pinOut", el) == 0) {
        pins_t* p = createPin(NULL);
        p->tmpID = pinOut.tmpID;
        p->pin = pinOut.pin;
        currentCircuit->pinsOut = g_list_append(currentCircuit->pinsOut, p);
    }
}

// presents a gtk file dialog for loading
void loadCircuit(circuit_t* c, const char* fileName)
{
    currentCircuit = c;
    clearCircuit(c);

    hash = g_hash_table_new(g_direct_hash, g_direct_equal);

    XML_Parser p = XML_ParserCreate(NULL);

    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }

    FILE *fp;
    fp = fopen(fileName, "r");
    if (!fp) return;

    XML_SetElementHandler(p, start, end);

    for (;;) {
        int done;
        int len;

        len = (int)fread(Buff, 1, BUFFSIZE, fp);

        if (ferror(stdin)) {
            fprintf(stderr, "Read error\n");
            exit(-1);
        }

        done = feof(fp);

        if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
            fprintf(stderr,
                    "Parse error at line %lu:\n%s\n",
                    XML_GetCurrentLineNumber(p),
                    XML_ErrorString(XML_GetErrorCode(p)));
            exit(-1);
        }

        if (done) {
            break;
        }
    }

    GList* i;
    for (i = currentCircuit->pinsIn; i!=NULL; i = i->next) {
        pins_t* p = (pins_t*)i->data;
        node_t* n = g_hash_table_lookup(hash, GINT_TO_POINTER(p->tmpID));
        p->node = n;
    }

    for (i = currentCircuit->pinsOut; i!=NULL; i = i->next) {
        pins_t* p = (pins_t*)i->data;
        node_t* n = g_hash_table_lookup(hash, GINT_TO_POINTER(p->tmpID));
        p->node = n;
    }

    XML_ParserFree(p);
    g_hash_table_destroy(hash);
    findSrcTargets(currentCircuit);

}

