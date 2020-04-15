#include <gtk/gtk.h>
#include "gtkLabel.h"

GtkWidget *boldLabel( GtkWidget *label ) {

    /* Set bold font for headerLeftLabel*/
    PangoAttrList *pangoAttrList = pango_attr_list_new();
    PangoAttribute *pangoAttribute = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
    pango_attr_list_insert ( pangoAttrList, pangoAttribute ); /* Set bold type*/
    gtk_label_set_attributes ( GTK_LABEL(label), pangoAttrList ) ;

    return label;
}


