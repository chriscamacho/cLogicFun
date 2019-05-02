
extern vec2_t ioPoints[16];
//extern int currentID;
//extern GList* nodeList;

extern char typeNames[9][8];
extern char invTypeNames[9][8];

extern GdkPixbuf* typeImg[9];
extern GdkPixbuf* invTypeImg[9];

// each node can optionally invert its output
enum nodeType {
    n_const = 0,
    n_not,
    n_and,
    n_or,
    n_xor,
    n_in,
    n_out,
    n_src,
    n_dst
};


typedef struct output_s {
    //int index;
    //void* wire;
    gboolean highlight;
} output_t;

typedef struct input_s {
    gboolean highlight;     // highlighted by gui (mouse hover)
    void* wire;
    gboolean state;
} input_t;

typedef struct node_s {
    guint id;
    vec2_t pos;
    double rotation;
    double width, height;
    enum nodeType type;

    int maxInputs;  // max physicsal attachment points
    input_t inputs[8];
    //gboolean inputStates[8];

    int maxOutputs;
    // only using 1 output - reserving others for nodes that are embedded circuits
    output_t outputs[8];

    // usually a list of wires except for n_src which
    // uses it as a list of nodes
    GList* outputList;

    gboolean state;
    gboolean stateBuffer[8]; // for latency
    char text[80];
    gboolean invert;
    int latency;
} node_t;

node_t* addNode(circuit_t* cir, enum nodeType tp, double x, double y);
void freeNode(circuit_t*, node_t* n);
void drawNode(cairo_t *cr, node_t* n);
gboolean pointInNode(double x, double y, node_t* n);
int pointInIo(double x, double y, node_t* n);
void calcIoPoints();
void clearCircuit(circuit_t* cir);
void updateLogic(circuit_t* cir);
void findSrcTargets(circuit_t* cir);

