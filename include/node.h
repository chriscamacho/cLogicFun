
extern vec2_t ioPoints[16];
extern int currentID;
extern GList* nodeList;

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
    gboolean highlight;
    void* wire;
} input_t;

typedef struct node_s {
    int id;
    vec2_t pos;
    double rotation;
    double width, height;
    enum nodeType type;
    int maxInputs;
    int maxOutputs;
    gboolean invert;
    gboolean state;
    // only using 1 output - reserving others for nodes that are embedded circuits
    output_t outputs[8];

    GList* outputWires;
    GList* srcOutputs;

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
void findSrcTargets();

