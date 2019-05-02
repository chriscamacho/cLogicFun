
//extern GList* wireList;

typedef struct wire_s {
    guint id;
    node_t* parent; // where it comes from
    node_t* target; //
    int outIndex; // which output is it connected to on the parent
    int inIndex; // which input does it connect to on the target
    vec2_t sp, ep, cp1, cp2; // start, end and two control points
    double colourR, colourG, colourB;
    gboolean state;
} wire_t;

void drawWires(cairo_t* cr, circuit_t* cir, double zoom);
wire_t* addWire(circuit_t* cir);
void updateWire(wire_t* w);
void deleteWire(circuit_t* cir, wire_t* w);
void propagateWires(circuit_t* cir);

