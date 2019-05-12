
typedef struct pins_s {
    node_t* node; // node the pin is connected to (sub circuit input or output)
    guint pin; // which pin
    guint tmpID;    // used by the loader because we don't know all the node pointers
                    // until after they are loaded
} pins_t;

void initPinsWin(GtkBuilder* builder, GtkWidget* mainWin);
void showPinsWindow(circuit_t* cir);
pins_t* createPin(node_t* n);
void freePin(pins_t* p);
