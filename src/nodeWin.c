
#include <gtk/gtk.h>
#include "vec2.h"
#include "circuit.h"
#include "node.h"
#include "wire.h"
#include "nodeWin.h"
#include <strings.h>

GtkWidget* nodeWindow;
GtkWidget* nodeWinType;
GtkWidget* nodeWinRotation;
GtkWidget* nodeWinInvert;
GtkWidget* nodeWinText;
GtkWidget* nodeWinLatency;

node_t* currentNode;
circuit_t* currentCircuit;

gboolean onDelete(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    guint ty = currentNode->type;

    while (currentNode->outputList) {
        wire_t* w = (wire_t*)currentNode->outputList->data;
        currentNode->outputList = g_list_remove(currentNode->outputList, w);
        deleteWire(currentCircuit, w);
    }

    for (int i = 0; i < 8; i++) {
        if (currentNode->inputs[i].wire) {
            deleteWire(currentCircuit, currentNode->inputs[i].wire);
        }
    }

    freeNode(currentCircuit, currentNode);

    // TODO handle n_src and n_dst?
    if (ty == n_src || ty == n_dst) {
        findSrcTargets(currentCircuit);
    }

    gtk_widget_hide(nodeWindow);
    return FALSE;
}



gboolean onNodeWinCancel(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    gtk_widget_hide(nodeWindow);
    return FALSE;
}

gboolean onNodeWinOK(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    GList* it;

    setNodeText(currentCircuit, currentNode, gtk_entry_get_text((GtkEntry*)nodeWinText));

    // have to make sure all node types have unique names for hash map txt > node

    for (it = currentCircuit->nodeList; it; it = it->next) {
        node_t* n = (node_t*)it->data;
        if (n==currentNode || strlen(n->p_text)==0) {
            continue;
        }
        if (strcasecmp(n->p_text, currentNode->p_text) == 0) {
            gtk_entry_set_text((GtkEntry*)nodeWinText,"");
             setNodeText(currentCircuit, currentNode, "");
            gtk_entry_set_placeholder_text((GtkEntry*)nodeWinText,"Needs to be unique");
            return FALSE;
        }
    }

    currentNode->invert = gtk_toggle_button_get_active((GtkToggleButton*)nodeWinInvert);
    double r = atof(gtk_entry_get_text((GtkEntry*)nodeWinRotation)) * D2R;
    currentNode->rotation = r;
    currentNode->latency = gtk_spin_button_get_value((GtkSpinButton*)nodeWinLatency)-1;
    // because feedback can do odd things to the state buffer!
    for (int i =0;i<8;i++) {
        currentNode->stateBuffer[i]=currentNode->state;
    }

    // TODO if n_src or n_dst do linkups
    findSrcTargets(currentCircuit);
    gtk_widget_hide(nodeWindow);
    return TRUE;
}

void initNodeWin(GtkBuilder *builder, GtkWidget* mainWin)
{
    nodeWindow = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWindow"));
    gtk_window_set_transient_for((GtkWindow*)nodeWindow, (GtkWindow*)mainWin);
    nodeWinType = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinType"));
    nodeWinRotation = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinRotation"));
    nodeWinInvert = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinInvert"));
    nodeWinText = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinText"));
    nodeWinLatency = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinLatency"));
    gtk_entry_set_activates_default ((GtkEntry*)nodeWinRotation, TRUE);
}

void showNodeWindow(circuit_t* cir, node_t* n)
{
    currentCircuit = cir;
    char degStr[80];
    currentNode = n;
    if (n->invert) {
        gtk_label_set_text((GtkLabel*)nodeWinType, invTypeNames[n->type]);
    } else {
        gtk_label_set_text((GtkLabel*)nodeWinType, typeNames[n->type]);
    }
    gtk_toggle_button_set_active((GtkToggleButton*)nodeWinInvert, n->invert);
    sprintf(degStr, "%f", n->rotation * R2D);
    gtk_entry_set_text((GtkEntry*)nodeWinRotation, degStr);
    gtk_entry_set_text((GtkEntry*)nodeWinText, n->p_text);
    gtk_spin_button_set_value((GtkSpinButton*)nodeWinLatency, n->latency+1);
    if (n->type == n_src || n->type == n_dst
        || n->type == n_in || n->type == n_out) {
        gtk_widget_set_sensitive(nodeWinInvert, FALSE);
        gtk_widget_set_sensitive(nodeWinLatency, FALSE);
    } else {
        gtk_widget_set_sensitive(nodeWinInvert, TRUE);
        gtk_widget_set_sensitive(nodeWinLatency, TRUE);
        gtk_entry_set_placeholder_text((GtkEntry*)nodeWinText,"");
    }
    gtk_widget_show(nodeWindow);
    // test of getNodeFromText
    //if (strlen(n->p_text)!=0) {
    //    printf("node text %s txt to node->p_text %s\n", n->p_text, getNodeFromText(cir,n->p_text)->p_text);
    //}
}
