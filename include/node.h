
extern vec2_t ioPoints[16];

extern char typeNames[10][8];
extern char invTypeNames[10][8];

extern GdkPixbuf* typeImg[10];
extern GdkPixbuf* invTypeImg[10];

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
    n_dst,
    n_sub
};


// was more in here...
typedef struct output_s {
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
    input_t inputs[8]; // consider list instead

    int maxOutputs;
    // only using 1 output - reserving others for nodes that are embedded circuits
    output_t outputs[8]; // consider list instead

    // usually a list of wires except for n_src which
    // uses it as a list of nodes
    GList* outputList;

    gboolean state;
    gboolean stateBuffer[8]; // for latency
    char p_text[80]; //  p_ "private" do not change directly
    gboolean invert;
    int latency;

    circuit_t* circuit; // only for n_sub
} node_t;

// adds a new node to a circuit
node_t* addNode(circuit_t* cir, enum nodeType tp, double x, double y);

// removes the node from a circuit and frees its memory
void freeNode(circuit_t*, node_t* n);

void drawNode(cairo_t *cr, node_t* n);

// is the point inside a node
gboolean pointInNode(double x, double y, node_t* n);

// is the point inside a nodes IO attachment point
int pointInIo(double x, double y, node_t* n);

// TODO better doing this on the fly?
void calcIoPoints();

// set a nodes text keeping text to node hashmap up to date
void setNodeText(circuit_t* c, node_t* n, const char* tx);

node_t* getNodeFromText(circuit_t* cir, const char* n);
