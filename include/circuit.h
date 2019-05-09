
// encapsulates the main and sub circuits

typedef struct circuit_s {
    GList* nodeList;
    GList* wireList;
    GList* pinsIn;
    GList* pinsOut;
    GHashTable* txtHash;
} circuit_t;

// allocates memory for a circuit struct
circuit_t* createCircuit();

// free's the circuits allocated memory
void freeCircuit(circuit_t* c);

// removes all the wires and nodes
void clearCircuit(circuit_t* cir);

// searches all used ID's looking for an unused one
guint getNextID(circuit_t* c);

// does a simulation tick for a circuit
void updateLogic(circuit_t* cir);

// matches dst node text with src node text creating
// list of src nodes for each dst node
void findSrcTargets(circuit_t* cir);
