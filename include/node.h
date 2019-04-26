#ifndef NODE_H
#define NODE_H

extern vec2_t ioPoints[16];
extern int currentID;
extern GList* nodeList;

extern char typeNames[7][8];
extern char invTypeNames[7][8];

extern GdkPixbuf* typeImg[7];
extern GdkPixbuf* invTypeImg[7];

// each node can optionally invert its output
enum nodeType {
    n_const = 0,
    n_not,
    n_and,
    n_or,
    n_xor,
    n_in,
    n_out
};


typedef struct output_s {
    int index;
    void* wire;
    gboolean highlight;
} output_t;

typedef struct input_s {
    int index;
    gboolean highlight;
    void* wire;
} input_t;

// each node has max 4 in/outputs
typedef struct node_s {
    int id;
    vec2_t pos;
    double rotation;
    enum nodeType type;
    int maxInputs;
    int maxOutputs;
    gboolean invert;
    gboolean state;
    // only using 1 output - reserving others for nodes that are embedded circuits
    output_t outputs[8];
    GList* outputWires;
    gboolean inputStates[8];
    input_t inputs[8];
    char text[80];
    gboolean stateBuffer[8];
    int latency;
} node_t;

node_t* addNode(enum nodeType tp, double x, double y);
void freeNode(node_t* n);
void drawNode(cairo_t *cr, node_t* n);
gboolean pointInNode(double x, double y, node_t* n);
int pointInIo(double x, double y, node_t* n);
void calcIoPoints();
void clearCircuit();
void updateLogic();

#endif // NODE_H
