#ifndef NODE_H
#define NODE_H

extern vec2_t ioPoints[8];
extern int currentID;
extern GSList* nodeList;

extern char typeNames[7][8];
extern char invTypeNames[7][8];

extern GdkPixbuf* typeImg[7];
extern GdkPixbuf* invTypeImg[7];

// each node can optionally invert its output
enum nodeType {
    n_split = 0,
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
    output_t outputs[4];
    gboolean inputStates[4];
    input_t inputs[4];
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
