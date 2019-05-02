
typedef struct circuit_s {
    guint nextID; // next free ID for this circuit
    GList* nodeList;
    GList* wireList;
} circuit_t;

circuit_t* createCircuit();
void freeCircuit(circuit_t* c);
