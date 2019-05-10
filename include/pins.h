
typedef struct pins_s {
    node_t* node; // node the pin is connected to (sub circuit input or output)
    gboolean isInput; //
    guint pin; // which pin
} pins_t;

void initPinsWin(GtkBuilder* builder, GtkWidget* mainWin);
void showPinsWindow(circuit_t* cir);
pins_t* createPin(node_t* n, gboolean input);
void freePin(pins_t* p);
