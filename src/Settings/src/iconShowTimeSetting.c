#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "settingWindowData.h"
#include "panel.h"
#include "gtkLabel.h"
#include "useful.h"
#include "configControl.h"

gchar * getScaleValue( GtkScale *scale, gdouble value, gpointer data ) {

    /* return g_strdup_printf ("\%0.*g", gtk_scale_get_digits (scale), value ); */
    return g_strdup_printf ("%.0f ms", value );
}

void set_to_default_delay_time_cb ( 
        GtkSwitch *switchButton,
        DelaySettingWindowData *delaySettingWindowData ) {

    gtk_range_set_value ( GTK_RANGE(delaySettingWindowData->delayScale), 1200 );
    gtk_widget_queue_draw ( (GtkWidget*)switchButton );
}

void scale_value_changed_call_back ( GtkRange *delayScale, SettingWindowData *settingWindowData ) {

    int value = 0;
    char str[10];
    value = gtk_range_get_value ( delayScale );
    settingWindowData->delaySettingWindowData->scaleValue = value;
    writeToConfig ( "Icon-Show-Time", int2str(value, str) );
    //gtk_button_set_label ( (GtkButton*)settingWindowData->button, g_strdup_printf ( "%d", value ) );
}


void iconShowTimeSetting( SettingWindowData *settingWindowData  ) {

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)settingWindowData->contentScrollWindow,
            remove_widget,
            (settingWindowData->contentScrollWindow)
            );

    static DelaySettingWindowData delaySettingWindowData;
    memset ( &delaySettingWindowData, '\0', sizeof(delaySettingWindowData) );
    settingWindowData->delaySettingWindowData = &delaySettingWindowData;

    GtkWidget *scrollWindow = settingWindowData->contentScrollWindow;
    GtkWidget *delayScale;
    GtkWidget *hBox;
    char readConfigBuf[52] = { '\0' };
    int defaultScaleValue = 1200;
    int getValue = 0;

    /* New vertical box*/
    hBox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 5 );
    gtk_container_add ( GTK_CONTAINER(scrollWindow), hBox );
    gtk_widget_set_margin_start ( hBox, MARGIN_START );
    gtk_widget_set_margin_end ( hBox, MARGIN_END );
    gtk_widget_set_margin_top ( hBox, MARGIN_TOP );

    /* New ListBox*/
    GtkWidget *listBox = gtk_list_box_new();
    delaySettingWindowData.listBox = listBox;

    /* New Title Of Scale*/
    GtkWidget *explainGrid = gtk_grid_new();
    gtk_box_pack_start ( GTK_BOX(hBox), explainGrid, 0,0,0 );
    gtk_grid_attach ( GTK_GRID(explainGrid),
            boldLabel(gtk_label_new("Icon Show Time After Popup")), 0, 0, 1, 1);
    gtk_box_pack_start ( GTK_BOX(hBox), listBox, 0, 0, 0 );

    /* New scale*/
    delayScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,5000, 1);
    GtkWidget *delayScaleBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request ( delayScaleBox, 450, LISTBOX_ROW_HEIGHT );

    delaySettingWindowData.delayScale = delayScale;

    getValue = str2int ( readFromConfig ( "Icon-Show-Time", readConfigBuf ) );
    getValue = getValue == 0 ? defaultScaleValue : getValue;
    gtk_range_set_value ( GTK_RANGE(delayScale), getValue);

    /* gtk_scale_set_draw_value ( GTK_SCALE(delayScale), true ); */
    gtk_scale_set_value_pos ( GTK_SCALE(delayScale), 1);
    /* gtk_scale_set_digits ( GTK_SCALE(delayScale), 0 ); */
    g_object_set(delayScale, "width-request", 450, NULL);
    gtk_list_box_insert ( GTK_LIST_BOX(listBox), delayScaleBox, -1 );

    gtk_widget_set_vexpand ( delayScaleBox, true );
    gtk_widget_set_hexpand ( delayScale, true );
    gtk_widget_set_valign ( delayScaleBox, GTK_ALIGN_CENTER );
    gtk_widget_set_valign ( delayScale, GTK_ALIGN_CENTER );
    gtk_widget_set_halign ( delayScale, GTK_ALIGN_CENTER );

    gtk_widget_set_margin_start ( delayScale, 0 );
    gtk_box_pack_start ( GTK_BOX(delayScaleBox), delayScale, 1, 1, 1 );

    /* New setToDefault widget*/
    /* GtkWidget *setToDefaultButton = gtk_switch_new(); */
    GtkWidget *setToDefaultButton = gtk_button_new_with_label ("SET");
    GtkWidget *setToDefaultExplainLabel = (gtk_label_new ( "Set To Default" ));
    gtk_widget_set_size_request ( setToDefaultButton, 100, -1 );
    GtkWidget *setToDefaultGrid = gtk_grid_new();

    /* Separator*/
    gtk_list_box_insert ( GTK_LIST_BOX(listBox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), -1 );

    /* setToDefault*/
    gtk_list_box_insert ( GTK_LIST_BOX(listBox), setToDefaultGrid, -1 );

    GtkWidget *boxInSetToDefaultGirdRight = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );

    delaySettingWindowData.boxInSetToDefaultGirdRight = boxInSetToDefaultGirdRight;

    gtk_widget_set_size_request ( setToDefaultGrid, 450, LISTBOX_ROW_HEIGHT );
    gtk_widget_set_margin_start ( setToDefaultGrid, LISTBOXROW_MARGIN_START );
    gtk_grid_attach ( GTK_GRID(setToDefaultGrid), setToDefaultExplainLabel , 0, 0, 1, 1 );
    gtk_grid_attach ( GTK_GRID(setToDefaultGrid), boxInSetToDefaultGirdRight , 1, 0, 1, 1 );
    gtk_box_pack_start ( GTK_BOX(boxInSetToDefaultGirdRight), setToDefaultButton, 0, 0, 0 );
    gtk_grid_set_column_spacing (  GTK_GRID(setToDefaultGrid), 250 );
    gtk_widget_set_margin_start ( setToDefaultExplainLabel, 9 );
    gtk_widget_set_margin_end ( setToDefaultButton, LISTBOXROW_MARGIN_END );

    gtk_widget_set_vexpand ( setToDefaultExplainLabel, true );
    gtk_widget_set_hexpand ( setToDefaultExplainLabel, false );
    gtk_widget_set_valign ( setToDefaultGrid, GTK_ALIGN_CENTER );
    gtk_widget_set_halign ( setToDefaultGrid, GTK_ALIGN_CENTER );
    gtk_widget_set_halign ( boxInSetToDefaultGirdRight, GTK_ALIGN_END );
    gtk_widget_set_valign ( boxInSetToDefaultGirdRight, GTK_ALIGN_CENTER );

    inline void disable_selectable_activatable( GtkWidget *widget, gpointer data ) {
        gtk_list_box_row_set_selectable ( GTK_LIST_BOX_ROW(widget), false );
        gtk_list_box_row_set_activatable ( GTK_LIST_BOX_ROW(widget), false );
    }

    /* Set all listBoxRow unSeletable and unActivatable*/
    gtk_container_forall ( 
            GTK_CONTAINER(listBox),
            disable_selectable_activatable,
            NULL
            );

    g_signal_connect ( delayScale, "value-changed", G_CALLBACK(scale_value_changed_call_back), settingWindowData);
    g_signal_connect ( delayScale, "format-value", G_CALLBACK(getScaleValue), NULL );
     g_signal_connect ( setToDefaultButton, "clicked", G_CALLBACK(set_to_default_delay_time_cb), &delaySettingWindowData );

    gtk_widget_show_all ( scrollWindow );
}

