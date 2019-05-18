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

typedef struct nodeData_s {
    node_t newNode;
    wire_t newWire;
    pins_t pinIn,pinOut;

    int pid, tid;
    char* tmpFilename;
    circuit_t* circuit;
} nodeData_t;




//circuit_t* currentCircuit;


// as each xml tag is first encountered the callback is triggered
static void XMLCALL start(void *data, const XML_Char *el, const XML_Char **attr)
{
    int i;
    //(void)data;
    (void)el;
    nodeData_t* nd = (nodeData_t*)data;

    for (i = 0; attr[i]; i += 2) {

        if (strcasecmp("pinIn", el) == 0) {
            if (strcasecmp("destID", attr[i]) == 0) {
                nd->pinIn.tmpID = atoi(attr[i + 1]);
            }
            if (strcasecmp("pin", attr[i]) == 0) {
                nd->pinIn.pin = atoi(attr[i + 1]);
            }
        }

        if (strcasecmp("pinOut", el) == 0) {
            if (strcasecmp("destID", attr[i]) == 0) {
                nd->pinOut.tmpID = atoi(attr[i + 1]);
            }
            if (strcasecmp("pin", attr[i]) == 0) {
                nd->pinOut.pin = atoi(attr[i + 1]);
            }
        }


        if (strcasecmp("node", el) == 0) {
            if (strcasecmp("nodeID", attr[i]) == 0) {
                nd->newNode.id = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("pos", el) == 0) {
            if (strcasecmp("x", attr[i]) == 0) {
                nd->newNode.pos.x = atoi(attr[i + 1]);
            }
            if (strcasecmp("y", attr[i]) == 0) {
                nd->newNode.pos.y = atoi(attr[i + 1]);
            }
            if (strcasecmp("rot", attr[i]) == 0) {
                nd->newNode.rotation = atof(attr[i + 1]);
            }
            if (strcasecmp("filename", attr[i]) == 0) {
                nd->tmpFilename = malloc(strlen(attr[i+1])+1);
                strcpy(nd->tmpFilename, attr[i+1]);
            }
        }
        if (strcasecmp("logic", el) == 0) {
            if (strcasecmp("type", attr[i]) == 0) {
                nd->newNode.type = atoi(attr[i + 1]);
            }
            if (strcasecmp("inv", attr[i]) == 0) {
                nd->newNode.invert = atoi(attr[i + 1]);
            }
            if (strcasecmp("latency", attr[i]) == 0) {
                nd->newNode.latency = atoi(attr[i + 1]);
            }
            if (strcasecmp("state", attr[i]) == 0) {
                nd->newNode.state = atoi(attr[i + 1]);
            }

        }
        if (strcasecmp("io", el) == 0) {
            if (strcasecmp("maxIn", attr[i]) == 0) {
                nd->newNode.maxInputs = atoi(attr[i + 1]);
            }
            if (strcasecmp("maxOut", attr[i]) == 0) {
                nd->newNode.maxOutputs = atoi(attr[i + 1]);
            }
        }

        if (strcasecmp("wire", el) == 0) {
            if (strcasecmp("wireID", attr[i]) == 0) {
                nd->newWire.id = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("parent", el) == 0) {
            if (strcasecmp("pid", attr[i]) == 0) {
                nd->pid = atoi(attr[i + 1]);
            }
            if (strcasecmp("pindex", attr[i]) == 0) {
                nd->newWire.outIndex = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("target", el) == 0) {
            if (strcasecmp("tid", attr[i]) == 0) {
                nd->tid = atoi(attr[i + 1]);
            }
            if (strcasecmp("tindex", attr[i]) == 0) {
                nd->newWire.inIndex = atoi(attr[i + 1]);
            }
        }
        if (strcasecmp("colour", el) == 0) {
            if (strcasecmp("r", attr[i]) == 0) {
                nd->newWire.colourR = atof(attr[i + 1]);
            }
            if (strcasecmp("g", attr[i]) == 0) {
                nd->newWire.colourG = atof(attr[i + 1]);
            }
            if (strcasecmp("b", attr[i]) == 0) {
                nd->newWire.colourB = atof(attr[i + 1]);
            }
        }
        if (strcasecmp("label", el) == 0) {
            if (strcasecmp("text", attr[i]) == 0) {
                // newNode never directly added to circuit so safe to directly modify
                strcpy(nd->newNode.p_text, attr[i + 1]);
            }
        }
    }

}

// when the end of the xml tag is encountered this is called
// the temporary node or wire is transfered to the circuit,
static void XMLCALL
end(void *data, const XML_Char *el)
{
    //(void)data;
    (void)el;
    //circuit_t* currentCircuit = (circuit_t*)data;
    nodeData_t* nd = (nodeData_t*)data;

    if (strcasecmp("node", el) == 0) {
        // add node from newNode
        node_t* n = addNode(nd->circuit, nd->newNode.type, nd->newNode.pos.x, nd->newNode.pos.y);
        n->id = nd->newNode.id;
        g_hash_table_insert(nd->circuit->idHash, GINT_TO_POINTER(n->id), n);
        n->rotation = nd->newNode.rotation;
        n->invert = nd->newNode.invert;
        n->maxInputs = nd->newNode.maxInputs;
        n->maxOutputs = nd->newNode.maxOutputs;
        n->latency = nd->newNode.latency;
        n->state = nd->newNode.state;
        for (int i=0; i<8; i++) {
            n->stateBuffer[i] = n->state;
        }
        setNodeText(nd->circuit, n, &nd->newNode.p_text[0]);
        if (nd->tmpFilename && n->type == n_sub) {
            n->circuit = createCircuit();
            n->circuit->filename = malloc(strlen(nd->tmpFilename)+1);
            strcpy(n->circuit->filename, nd->tmpFilename);
            free(nd->tmpFilename);
            nd->tmpFilename = 0;
        }


    }

    if (strcasecmp("wire", el) == 0) {
        // add wire from newWire
        wire_t* w = addWire(nd->circuit);
        w->id = nd->newWire.id;
        w->parent = g_hash_table_lookup(nd->circuit->idHash, GINT_TO_POINTER(nd->pid));
        w->target = g_hash_table_lookup(nd->circuit->idHash, GINT_TO_POINTER(nd->tid));
        w->inIndex = nd->newWire.inIndex;
        w->outIndex = nd->newWire.outIndex;
        w->colourR = nd->newWire.colourR;
        w->colourG = nd->newWire.colourG;
        w->colourB = nd->newWire.colourB;

        w->parent->outputList = g_list_append(w->parent->outputList, w);
        w->target->inputs[w->inIndex].wire = w;
    }

    if (strcasecmp("pinIn", el) == 0) {
        pins_t* p = createPin(NULL);
        p->tmpID = nd->pinIn.tmpID;
        p->pin = nd->pinIn.pin;
        nd->circuit->pinsIn = g_list_append(nd->circuit->pinsIn, p);
    }

    if (strcasecmp("pinOut", el) == 0) {
        pins_t* p = createPin(NULL);
        p->tmpID = nd->pinOut.tmpID;
        p->pin = nd->pinOut.pin;
        nd->circuit->pinsOut = g_list_append(nd->circuit->pinsOut, p);
    }
}

// presents a gtk file dialog for loading
void loadCircuit(circuit_t* c, const char* fileName)
{
    clearCircuit(c);

    c->idHash = g_hash_table_new(g_direct_hash, g_direct_equal);

    XML_Parser p = XML_ParserCreate(0);

    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }

    FILE *fp;
    fp = fopen(fileName, "r");
    if (!fp) return;

    XML_SetElementHandler(p, start, end);
    nodeData_t* nd = malloc(sizeof(nodeData_t));
    nd->circuit = c;
    XML_SetUserData(p, (void*)nd);

    char Buff[BUFFSIZE];
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


    if (c->filename!=0) {
        free(c->filename);
        c->filename=0;
    }
    if (c->path!=0) {
        free(c->path);
        c->path=0;
    }

    c->filename = malloc(strlen(fileName)+1);
    strcpy(c->filename, fileName);
    char path[1024];
    strcpy(path,c->filename);
    char* f = g_strrstr(path,"/");
    f[0]='\0';
    c->path = malloc(strlen(path)+1);
    strcpy(c->path, path);

    GList* i;
    c->nIns = 0;
    for (i = c->pinsIn; i!=NULL; i = i->next) {
        pins_t* p = (pins_t*)i->data;
        node_t* n = g_hash_table_lookup(c->idHash, GINT_TO_POINTER(p->tmpID));
        p->node = n;
        c->nIns++;
    }

    c->nOuts = 0;
    for (i = c->pinsOut; i!=NULL; i = i->next) {
        pins_t* p = (pins_t*)i->data;
        node_t* n = g_hash_table_lookup(c->idHash, GINT_TO_POINTER(p->tmpID));
        p->node = n;
        c->nOuts++;
    }


    // load up all sub circuits
    for (GList* it = c->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_sub) {
            //if (n->circuit == 0) {
            //    n->circuit = createCircuit();
            //}
            n->circuit->path = malloc(strlen(c->path)+1);
            strcpy(n->circuit->path, c->path);
            char fullpath[1024];
            sprintf(fullpath,"%s/%s",n->circuit->path,n->circuit->filename);
            loadCircuit(n->circuit, fullpath);
        }
    }

    XML_ParserFree(p);
    g_hash_table_destroy(c->idHash);
    findSrcTargets(c);

    free(nd);

}

