#include "gtk/gtk.h"
#include "settingWindowData.h"
#include "panel.h"


void remove_widget ( GtkWidget *widget, void *container ) {

    gtk_container_remove ( GTK_CONTAINER(container), widget );
}
