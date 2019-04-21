
#include <gtk/gtk.h>
#include "vec2.h"
#include "node.h"
#include "wire.h"
#include "nodeWin.h"

GtkWidget* nodeWindow;
GtkWidget* nodeWinType;
GtkWidget* nodeWinRotation;
GtkWidget* nodeWinInvert;
node_t* currentNode;

gboolean onDelete(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    for (int i=0; i<4; i++)
    {
        if (currentNode->inputs[i].wire)
        {
            deleteWire(currentNode->inputs[i].wire);
        }
        if (currentNode->outputs[i].wire)
        {
            deleteWire(currentNode->outputs[i].wire);
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
    currentNode->invert = gtk_toggle_button_get_active((GtkToggleButton*)nodeWinInvert);
    double r = atof(gtk_entry_get_text((GtkEntry *)nodeWinRotation)) * D2R;
    currentNode->rotation = r;
    gtk_widget_hide(nodeWindow);
    return FALSE;
}

void initNodeWin(GtkBuilder *builder) 
{
    nodeWindow = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWindow"));
    nodeWinType = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinType"));
    nodeWinRotation = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinRotation"));
    nodeWinInvert = GTK_WIDGET(gtk_builder_get_object(builder, "nodeWinInvert"));
    gtk_entry_set_activates_default ((GtkEntry*)nodeWinRotation, TRUE);
}

void showNodeWindow(node_t* n)
{
    char degStr[80];
    currentNode = n;
    gtk_label_set_text((GtkLabel*)nodeWinType, typeNames[n->type]);
    gtk_toggle_button_set_active((GtkToggleButton*)nodeWinInvert,n->invert);
    sprintf(degStr,"%f",n->rotation * R2D);
    gtk_entry_set_text((GtkEntry*)nodeWinRotation,degStr);
    gtk_widget_show(nodeWindow);
}
