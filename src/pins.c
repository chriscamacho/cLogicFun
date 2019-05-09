#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "pins.h"

// TODO split UI and basic "pin" functionality to different sources

pins_t* createPin(node_t* n, gboolean input)
{
    pins_t* p = malloc(sizeof(pins_t));
    p->isInput = input;
    p->node = n;
    return p;
}

void freePin(pins_t* p) // in case anything else gets added that needs handling
{
    free(p);
}

// yuk !
#define CUSTOM_TYPE_CELL_RENDERER_TEXT           (custom_cell_renderer_text_get_type())
#define CUSTOM_CELL_RENDERER_TEXT(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), CUSTOM_TYPE_CELL_RENDERER_TEXT, CustomCellRendererText))
#define CUSTOM_CELL_RENDERER_TEXT_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST((cls),    CUSTOM_TYPE_CELL_RENDERER_TEXT, CustomCellRendererTextClass))
#define CUSTOM_IS_CELL_RENDERER_TEXT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), CUSTOM_TYPE_CELL_RENDERER_TEXT))
#define CUSTOM_IS_CELL_RENDERER_TEXT_CLASS(cls)  (G_TYPE_CHECK_CLASS_TYPE((cls),    CUSTOM_TYPE_CELL_RENDERER_TEXT))
#define CUSTOM_CELL_RENDERER_TEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj),  CUSTOM_TYPE_CELL_RENDERER_TEXT, CustomCellRendererTextClass))

GType  custom_cell_renderer_text_get_type(void) G_GNUC_CONST;

typedef struct {
    GtkCellRendererText  renderer;
} CustomCellRendererText;

typedef struct {
    GtkCellRendererTextClass  parent_class;
} CustomCellRendererTextClass;

G_DEFINE_TYPE(CustomCellRendererText, custom_cell_renderer_text, GTK_TYPE_CELL_RENDERER_TEXT)

void custom_cell_renderer_text_render(GtkCellRenderer *cell,
                                      cairo_t *cr,
                                      GtkWidget *widget,
                                      const GdkRectangle *backarea,
                                      const GdkRectangle *cellarea,
                                      GtkCellRendererState state);

void custom_cell_renderer_text_class_init(CustomCellRendererTextClass *cls);
void custom_cell_renderer_text_init(CustomCellRendererText *renderer);
GtkCellRenderer *custom_cell_renderer_text_new(void);

void custom_cell_renderer_text_render(GtkCellRenderer *cell,
                                      cairo_t *cr,
                                      GtkWidget *widget,
                                      const GdkRectangle *backarea,
                                      const GdkRectangle *cellarea,
                                      GtkCellRendererState state)
{

    gchar *strval;
    gboolean bail = FALSE;
    g_object_get (cell, "text", &strval, NULL);
    //printf("renderer %s\n",strval);
    if (strlen(strval)>2) {
        bail=TRUE;
    }
    g_free (strval);
    if (bail) {
        return;
    }
    ((GtkCellRendererClass *)custom_cell_renderer_text_parent_class)->render(cell, cr, widget, backarea, cellarea, state);
}

void custom_cell_renderer_text_class_init(CustomCellRendererTextClass *cls)
{
    ((GtkCellRendererClass *)cls)->render = custom_cell_renderer_text_render;
    return;
}


void custom_cell_renderer_text_init(CustomCellRendererText *renderer)
{
    (void)renderer;
    return;
}


GtkCellRenderer *custom_cell_renderer_text_new(void)
{
    return g_object_new(CUSTOM_TYPE_CELL_RENDERER_TEXT, NULL);
}



GtkWidget* inputTreeView;
GtkWidget* outputTreeView;
GtkWidget* pinMapWindow;
GtkWidget* inputSpin;
GtkWidget* outputSpin;
circuit_t* currentCircuit;

int nInputs;
int nOutputs;
int mInputs; // max input candidates
int mOutputs;


#define UNUSED_PIN  100

enum
{
  COL_PIN = 0,
  COL_NAME,
  NUM_COLS
} ;

gboolean onPinCancel(GtkWidget* widget, gpointer data)
{
    (void)widget;
    (void)data;
    gtk_widget_hide(pinMapWindow);
    return FALSE;
}


gboolean onOK(GtkWidget* widget, gpointer data)
{
    (void)widget;
    (void)data;
    // clear out old pin defs
    while (currentCircuit->pinsIn) {
        pins_t* p = (pins_t*)currentCircuit->pinsIn->data;
        freePin(p);
    }
    while (currentCircuit->pinsOut) {
        pins_t* p = (pins_t*)currentCircuit->pinsOut->data;
        freePin(p);
    }
    g_list_free(currentCircuit->pinsIn);
    g_list_free(currentCircuit->pinsOut);



    return FALSE;
}



void rowDeleted(GtkTreeModel *tree_model,
               GtkTreePath  *path,
               gpointer      data)
{
    //printf("row deleted\n");
    (void)path;
    GtkTreeIter  it;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tree_model), &it);
    int i=0;
    int t = *(int*)data;
    while (valid)
    {
        if (i>=t) {
            i = UNUSED_PIN;
        }
        gtk_list_store_set(GTK_LIST_STORE(tree_model), &it, COL_PIN, i, -1);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_model), &it);
        i++;
    }
}


void inputSpinChange(GtkSpinButton *spinButton, GtkScrollType scroll, gpointer data)
{
    (void)scroll;
    (void)data;
    nInputs = gtk_spin_button_get_value_as_int(spinButton);
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(inputTreeView));
    rowDeleted(model, NULL, &nInputs); // change the pin values
}

void outputSpinChange(GtkSpinButton *spinButton, GtkScrollType scroll, gpointer data)
{
    (void)scroll;
    (void)data;
    nOutputs = gtk_spin_button_get_value_as_int(spinButton);
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(outputTreeView));
    rowDeleted(model, NULL, &nOutputs);
}

void initPinsWin(GtkBuilder* builder, GtkWidget* mainWin)
{
    inputTreeView = GTK_WIDGET(gtk_builder_get_object(builder, "inputTreeView"));
    outputTreeView = GTK_WIDGET(gtk_builder_get_object(builder, "outputTreeView"));
    pinMapWindow = GTK_WIDGET(gtk_builder_get_object(builder, "pinMapWindow"));
    inputSpin = GTK_WIDGET(gtk_builder_get_object(builder, "inputSpin"));
    outputSpin = GTK_WIDGET(gtk_builder_get_object(builder, "outputSpin"));
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(inputSpin), 1, 1);
    gtk_spin_button_set_increments(GTK_SPIN_BUTTON(outputSpin), 1, 1);

    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(inputTreeView), TRUE);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW(outputTreeView), TRUE);
    gtk_window_set_transient_for((GtkWindow*)pinMapWindow, (GtkWindow*)mainWin);


    GtkCellRenderer* renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (inputTreeView),
            -1, "Name", renderer, "text", COL_NAME, NULL);
    renderer = custom_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (inputTreeView),
            -1, "Pin", renderer, "text", COL_PIN, NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (outputTreeView),
            -1, "Name", renderer, "text", COL_NAME, NULL);
    renderer = custom_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (outputTreeView),
            -1, "Pin", renderer, "text", COL_PIN, NULL);
}

void addRow(GtkListStore* store, GtkTreeIter* iter, const char* name, int pin)
{

    gtk_list_store_append (store, iter);
    gtk_list_store_set (store, iter,
                      COL_NAME, name,
                      COL_PIN, pin,
                      -1);

}

void showPinsWindow(circuit_t* cir)
{

    GtkListStore* inStore;
    GtkListStore* outStore;
    GtkTreeIter iter;

    inStore = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);
    outStore = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);
    nInputs = 0;
    nOutputs = 0;
    GList* it;
    for (it = cir->nodeList; it!=NULL; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n->type == n_in) {
            addRow(inStore, &iter, n->text, UNUSED_PIN);
            mInputs++;
        }
        if (n->type == n_out) {
            addRow(outStore, &iter, n->text, UNUSED_PIN);
            mOutputs++;
        }
    }

    currentCircuit = cir;


    g_signal_connect (inStore, "row-deleted", G_CALLBACK (rowDeleted), &nInputs);
    gtk_tree_view_set_model (GTK_TREE_VIEW (inputTreeView), GTK_TREE_MODEL(inStore));
    g_object_unref (inStore);

    g_signal_connect (outStore, "row-deleted", G_CALLBACK (rowDeleted), &nOutputs);
    gtk_tree_view_set_model (GTK_TREE_VIEW (outputTreeView), GTK_TREE_MODEL(outStore));
    g_object_unref (outStore);

    gtk_spin_button_set_range(GTK_SPIN_BUTTON(inputSpin), 0, mInputs);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(outputSpin), 0, mOutputs);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(inputSpin), nInputs);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(outputSpin), nOutputs);

    gtk_widget_show(pinMapWindow);
}
