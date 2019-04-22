#include <gtk/gtk.h>

#include "vec2.h"
#include "xmlLoader.h"
#include "node.h"
#include "wire.h"

#include <expat.h>
#include <stdio.h>
#include <strings.h>


#define BUFFSIZE        8192
char Buff[BUFFSIZE];

node_t newNode;
wire_t newWire;

int pid, tid;

GHashTable* hash;


// as each xml tag is first encountered the callback is triggered
static void XMLCALL start(void *data, const XML_Char *el, const XML_Char **attr)
{
    int i;
    (void)data;
    (void)el;

    for (i = 0; attr[i]; i += 2) {
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
        node_t* n = addNode(newNode.type, newNode.pos.x, newNode.pos.y);
        n->id = newNode.id;
        g_hash_table_insert(hash, GINT_TO_POINTER(n->id), n);
        n->rotation = newNode.rotation;
        n->invert = newNode.invert;
        n->maxInputs = newNode.maxInputs;
        n->maxOutputs = newNode.maxOutputs;
        if (n->type == n_in) {
            // fudge to help some feedback circuits like latches settle
            n->state = TRUE;
        }
    }
    if (strcasecmp("wire", el) == 0) {
        // add wire from newWire
        wire_t* w = addWire();
        w->id = newWire.id;
        w->parent = g_hash_table_lookup(hash, GINT_TO_POINTER(pid));
        w->target = g_hash_table_lookup(hash, GINT_TO_POINTER(tid));
        w->inIndex = newWire.inIndex;
        w->outIndex = newWire.outIndex;
        w->colourR = newWire.colourR;
        w->colourG = newWire.colourG;
        w->colourB = newWire.colourB;

        w->parent->outputs[w->outIndex].wire = w;
        w->target->inputs[w->inIndex].wire = w;
    }
}

// presents a gtk file dialog for loading
void loadCircuit(const char* fileName)
{
    clearCircuit();

    hash = g_hash_table_new(g_direct_hash, g_direct_equal);

    XML_Parser p = XML_ParserCreate(NULL);

    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }

    FILE *fp;
    fp = fopen(fileName, "r");

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

    XML_ParserFree(p);
    g_hash_table_destroy(hash);
    
    // fudge to help some feedback circuits like latches settle
    GSList* it;
    for (it = nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_in) {
            n->state = FALSE;
        }
        propagateWires();
        updateLogic();
    }        
}

