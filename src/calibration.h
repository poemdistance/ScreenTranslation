#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__


void listenRelativeEvent(GtkWidget *button, WinData *win );

gboolean release_button (GtkWidget *widget, GdkEventKey *event, gpointer *data);
gboolean press_button (GtkWidget *widget, GdkEventKey *event, gpointer *data);
gboolean leave_button (GtkWidget *widget, GdkEventKey *event, gpointer *data);
gboolean enter_button (GtkWidget *widget, GdkEventKey *event, gpointer *data);

gboolean deal_motion_notify_event ( GtkWidget *widget, GdkEventMotion *event, gpointer *data);

#endif
