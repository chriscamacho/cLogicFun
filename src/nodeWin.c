
#include <gtk/gtk.h>
#include "vec2.h"
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

gboolean onDelete(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    
    // TODO handle n_src and n_dst
    if (currentNode->type == n_src || currentNode->type == n_dst) {
        // set text to something unique and invalid
        currentNode->text[0]=127;
        currentNode->text[1]=0;
        // rebuild label targets
        findSrcTargets();
    }

    while (currentNode->outputWires) {
        wire_t* w = (wire_t*)currentNode->outputWires->data;
        currentNode->outputWires = g_list_remove(currentNode->outputWires, w);
        deleteWire(w);
    }

    for (int i = 0; i < 8; i++) {
        if (currentNode->inputs[i].wire) {
            deleteWire(currentNode->inputs[i].wire);
        }
    }

    freeNode(currentNode);

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
    strcpy(currentNode->text, gtk_entry_get_text((GtkEntry*)nodeWinText));

    if (currentNode->type == n_src) {
        
        for (it = nodeList; it; it = it->next) {
            node_t* n = (node_t*)it->data;
            if (n==currentNode) {
                continue;
            }
            if (n->type == n_src) {
                if (strcasecmp(n->text, currentNode->text) == 0) {
                    gtk_entry_set_text((GtkEntry*)nodeWinText,"");
                    currentNode->text[0]=0;
                    gtk_entry_set_placeholder_text((GtkEntry*)nodeWinText,"Needs to be unique");
                    return FALSE;
                }
            }
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
    findSrcTargets();
    printf("?\n");
    gtk_widget_hide(nodeWindow);
    return TRUE;
}

void initNodeWin(GtkBuilder *builder)
{
    nodeWindow = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWindow"));
    nodeWinType = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinType"));
    nodeWinRotation = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinRotation"));
    nodeWinInvert = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinInvert"));
    nodeWinText = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinText"));
    nodeWinLatency = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinLatency"));
    gtk_entry_set_activates_default ((GtkEntry*)nodeWinRotation, TRUE);
}

void showNodeWindow(node_t* n)
{
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
    gtk_entry_set_text((GtkEntry*)nodeWinText, n->text);
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
}
