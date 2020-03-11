#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include <string.h>
#include "settingWindowData.h"
#include "panel.h"
#include "configControl.h"
#include "settingList.h"
#include "settingWindowData.h"
#include "expanduser.h"
#include "shortcutListener.h"
#include "gtkLabel.h"
#include "useful.h"
#include "printWithColor.h"

#define LABEL_POS_LEFT ( 0 )
#define LABEL_POS_RIGHT ( 1 )

GtkLabel *getLabelBySelectedRow ( SettingWindowData *swd, int label_pos );
const char *getLabelText ( GtkLabel *label );

static const guint forbidden_keyvals[] = {
    /* Navigation keys */
    GDK_KEY_Home,
    GDK_KEY_Left,
    GDK_KEY_Up,
    GDK_KEY_Right,
    GDK_KEY_Down,
    GDK_KEY_Page_Up,
    GDK_KEY_Page_Down,
    GDK_KEY_End,
    GDK_KEY_Tab,

    /* Return */
    GDK_KEY_KP_Enter,
    GDK_KEY_Return,

    GDK_KEY_Mode_switch
};

static const char whiteList[][128] = { 
    "Play-Audio",
    "Switch-Translation-Source", 
};

static void release_grab ( GdkDevice *grab_pointer, GtkWidget *window );

void toggleWindowSensitive ( SettingWindowData *settingWindowData, gboolean mode ) {

    gtk_widget_set_sensitive ( settingWindowData->window, mode );
}

void on_clear_button_click_cb ( 
        GtkWidget *button,
        SettingWindowData *swd
        ) {

    ShortcutSettingWindowData *sswd = swd->shortcutSettingWindowData;

    GtkWidget *box = gtk_widget_get_parent ( button );
    GtkWidget *grid = gtk_widget_get_parent ( box );
    GtkWidget *listBoxRow = gtk_widget_get_parent ( grid );

    sswd->selectedRow = (GtkListBoxRow*)listBoxRow;
    GtkLabel *label =  getLabelBySelectedRow ( swd, 0 );
    const char *text = getLabelText ( label );
    gchar keyName[64];
    strcpy ( keyName, text );
    strcat ( keyName, "-Shortcut" );;

    writeToConfig ( keyName, "Disable" );

    label = getLabelBySelectedRow ( swd, 1 );
    gtk_label_set_text ( label, "DISABLE" );

    gtk_widget_show ( (GtkWidget*)label );
}

void on_dialog_destroy_cb ( GtkWidget *dialog, SettingWindowData *settingWindowData ) {

    toggleWindowSensitive ( settingWindowData, TRUE );
    GdkDevice *grab_pointer = settingWindowData->shortcutSettingWindowData->grab_pointer;
    GtkWidget *window = settingWindowData->window;
    release_grab ( grab_pointer, window);
    gtk_widget_destroy ( dialog );
}


    static void
release_grab ( GdkDevice *grab_pointer, GtkWidget *window )
{
    if (grab_pointer)
    {
        gdk_seat_ungrab (gdk_device_get_seat (grab_pointer));
        grab_pointer = NULL;

        gtk_grab_remove (window);
    }
}


gboolean keyval_is_forbidden (guint keyval)
{
    guint i;

    for (i = 0; i < G_N_ELEMENTS(forbidden_keyvals); i++) {
        if (keyval == forbidden_keyvals[i])
            return TRUE;
    }

    return FALSE;
}


    gboolean
is_valid_accel ( int mask, int keyval )
{
    /* Unlike gtk_accelerator_valid(), we want to allow Tab when combined
     * with some modifiers (Alt+Tab and friends)
     */
    return gtk_accelerator_valid (keyval, mask) ||
        (keyval == GDK_KEY_Tab && mask != 0);
}

    gboolean
is_valid_binding ( int mask, int keycode, int keyval )
{
    if ((mask == 0 || mask == GDK_SHIFT_MASK) && keycode != 0)
    {
        guint keyval = keyval;

        if ((keyval >= GDK_KEY_a && keyval <= GDK_KEY_z)
                || (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z)
                || (keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9)
                || (keyval >= GDK_KEY_kana_fullstop && keyval <= GDK_KEY_semivoicedsound)
                || (keyval >= GDK_KEY_Arabic_comma && keyval <= GDK_KEY_Arabic_sukun)
                || (keyval >= GDK_KEY_Serbian_dje && keyval <= GDK_KEY_Cyrillic_HARDSIGN)
                || (keyval >= GDK_KEY_Greek_ALPHAaccent && keyval <= GDK_KEY_Greek_omega)
                || (keyval >= GDK_KEY_hebrew_doublelowline && keyval <= GDK_KEY_hebrew_taf)
                || (keyval >= GDK_KEY_Thai_kokai && keyval <= GDK_KEY_Thai_lekkao)
                || (keyval >= GDK_KEY_Hangul_Kiyeog && keyval <= GDK_KEY_Hangul_J_YeorinHieuh)
                || (keyval == GDK_KEY_space && mask == 0)
                || keyval_is_forbidden (keyval)) {
            return FALSE;
        }
    }
    return TRUE;
}

GtkLabel *getLabelBySelectedRow ( SettingWindowData *swd, int label_pos ) {

    ShortcutSettingWindowData *sswd = swd->shortcutSettingWindowData;
    GtkGrid *grid = (GtkGrid*)gtk_bin_get_child ( GTK_BIN(sswd->selectedRow) );
    GtkBox *box = (GtkBox*)gtk_grid_get_child_at ( GTK_GRID(grid), label_pos, 0 );
    GList *childList  = gtk_container_get_children ( GTK_CONTAINER(box) );
    GtkLabel *label = NULL;

    for ( GList *list = childList; list != NULL; list=list->next ) {

        if ( GTK_LABEL ( list->data ) ) {
            label  = GTK_LABEL(list->data);
            break;
        }
    }

    return label;
}

const char *getLabelText ( GtkLabel *label ) {

    return gtk_label_get_text ( label );
}

static gboolean is_no_need_to_check ( SettingWindowData *swd ) {

    const char *text = getLabelText ( 
            getLabelBySelectedRow ( swd, LABEL_POS_LEFT ) );
    for ( int i=0; i<sizeof(whiteList)/sizeof(whiteList[0]); i++ ) {
        if ( strcmp ( whiteList[i], text ) == 0 )
            return TRUE;
    }

    return FALSE;
}

void replace_button_click_cb ( GtkWidget *button,  SettingWindowData *swd ) {

    ShortcutSettingWindowData *sswd = swd->shortcutSettingWindowData;
    GtkLabel *label = getLabelBySelectedRow ( swd, LABEL_POS_LEFT );

    const char *labelText = getLabelText(label);
    gchar keyName[64];
    strcpy ( keyName, labelText );
    strcat ( keyName, "-Shortcut" );;

    writeToConfig ( keyName, sswd->receiveShortcut );

    label = getLabelBySelectedRow ( swd, LABEL_POS_RIGHT  );
    gtk_label_set_text ( label, upperCase(sswd->receiveShortcut) );
}

    static int
on_dialog_key_press_event_cb (GtkWidget   *dialog,
        GdkEventKey *event, SettingWindowData *swd)
{

    ShortcutSettingWindowData *sswd = swd->shortcutSettingWindowData;

    gboolean pass = FALSE;
    if ( is_no_need_to_check ( swd ) ) pass = TRUE;

    /* CcKeyboardShortcutEditor *self; */
    GdkModifierType real_mask;
    guint keyval_lower;

    real_mask = event->state & gtk_accelerator_get_default_mod_mask ();

    keyval_lower = gdk_keyval_to_lower (event->keyval);

    /* Normalise <Tab> */
    if (keyval_lower == GDK_KEY_ISO_Left_Tab)
        keyval_lower = GDK_KEY_Tab;

    /* Put shift back if it changed the case of the key, not otherwise. */
    if (keyval_lower != event->keyval)
        real_mask |= GDK_SHIFT_MASK;

    if (keyval_lower == GDK_KEY_Sys_Req &&
            (real_mask & GDK_MOD1_MASK) != 0)
    {
        /* HACK: we don't want to use SysRq as a keybinding (but we do
         * want Alt+Print), so we avoid translation from Alt+Print to SysRq */
        keyval_lower = GDK_KEY_Print;
    }

    /* GdkDevice *grab_pointer = sswd->grab_pointer; */
    /* GtkWidget *window = settingWindowData->window; */

    /* A single Escape press cancels the editing */
    if (!event->is_modifier && real_mask == 0 && keyval_lower == GDK_KEY_Escape)
    {
        /* release_grab ( grab_pointer, window); */
        on_dialog_destroy_cb ( sswd->dialog, swd );
        return GDK_EVENT_STOP;
    }

    /* Backspace disables the current shortcut */
    if (!event->is_modifier && real_mask == 0 && keyval_lower == GDK_KEY_BackSpace)
    {
        /* release_grab ( grab_pointer, window ); */
        /* on_dialog_destroy_cb ( sswd->dialog, swd ); */
        gtk_widget_show ( sswd->image );
        gtk_label_set_text ( (GtkLabel*)sswd->tipLabel, 
                "Press Esc to cancel or Backspace to disable the keyboard shortcut.");
        gtk_widget_show ( sswd->tipLabel );
        return GDK_EVENT_STOP;
    }

    if ( !pass && (real_mask == 0 || event->hardware_keycode == 0) ) {

        return GDK_EVENT_STOP;
    }

    /* gboolean custom_is_modifier = event->is_modifier; */
    gint keycode = event->hardware_keycode;
    gint keyval = keyval_lower;
    gint mask = real_mask;

    /* CapsLock isn't supported as a keybinding modifier, so keep it from confusing us */
    mask &= ~GDK_LOCK_MASK;

    if ( !pass && (! is_valid_binding ( mask, keycode, keyval ) \
                || !is_valid_accel(mask, keyval) \
                || event->is_modifier) ) {
        return GDK_EVENT_STOP;
    }

    printf("Key press event mask=%d keyval=%d \n", mask, keyval);

    gtk_widget_show ( sswd->replaceButton );
    gtk_widget_set_sensitive ( sswd->replaceButton, TRUE );

    gchar receive[32] = { '\0' };

    char *keystr = gdk_keyval_name( keyval );

    getModifiersMapping ( NULL );
    mask2str(mask, receive);

    strcat ( receive, keystr );

    boldLabel ( sswd->tipLabel );
    gtk_label_set_text ( (GtkLabel*)sswd->tipLabel, receive );

    printf("receive:%s\n", receive);
    strcpy ( sswd->receiveShortcut, receive );

    gtk_widget_hide ( sswd->image );
    gtk_widget_show ( sswd->tipLabel );

    return GDK_EVENT_STOP;
}

void grab_seat ( SettingWindowData *settingWindowData ) {

    GtkWidget *window = settingWindowData->window;
    GdkSeat *seat = gdk_display_get_default_seat (gdk_window_get_display (
                gtk_widget_get_window(window)));

    GdkGrabStatus status = gdk_seat_grab (seat,
            gtk_widget_get_window(window),
            GDK_SEAT_CAPABILITY_KEYBOARD,
            FALSE,
            NULL,
            NULL,
            NULL,
            NULL);

    if (status != GDK_GRAB_SUCCESS) {
        g_warning ("Grabbing keyboard failed");
    }

    GdkDevice *grab_pointer = gdk_seat_get_keyboard (seat);
    if (!grab_pointer)
        grab_pointer = gdk_seat_get_pointer (seat);

    if ( grab_pointer )
        settingWindowData->shortcutSettingWindowData->grab_pointer = grab_pointer;

    gtk_grab_add (window);
}

static void on_listbox_row_selected_cb (
        GtkListBox    *box,
        GtkListBoxRow *row,
        SettingWindowData *settingWindowData) {

    GtkBuilder *builder = gtk_builder_new ();
    GError *error = NULL;
    if ( gtk_builder_add_from_file (builder,
                expanduser("/home/$USER/.stran/cc-keyboard-shortcut-editor.ui"), &error) == 0)
        g_printerr ("Error loading file: %s\n", error->message);

    ShortcutSettingWindowData *swd = settingWindowData->shortcutSettingWindowData;
    swd->selectedRow = row;

    GObject *dialog = gtk_builder_get_object (builder, "dialog");
    swd->dialog = (GtkWidget*)dialog;
    gtk_window_set_title ( GTK_WINDOW(dialog), "Set shortcut" );
    gtk_window_set_default_size ( GTK_WINDOW(dialog) , 400, 400*0.618 );
    gtk_window_set_position ( GTK_WINDOW(dialog), GTK_WIN_POS_CENTER );
    gtk_widget_show ( (GtkWidget*)( dialog ) );

    /* Dialog windows should be set transient for the main application window they were spawned from.
     * This allows window managers to e.g. keep the dialog on top of the main window,
     * or center the dialog over the main window. */
    /* gtk_window_set_transient_for ( GTK_WINDOW(dialog) , (GtkWindow*)settingWindowData->window );; */

    GObject *replaceButton = gtk_builder_get_object ( builder, "replace_button" );
    GObject *addButton = gtk_builder_get_object ( builder, "add_button" );
    GObject *cancelButton = gtk_builder_get_object ( builder, "cancel_button" );
    GObject *setButton = gtk_builder_get_object ( builder, "set_button" );
    GObject *removeButton = gtk_builder_get_object ( builder, "remove_button" );
    GObject *image = gtk_builder_get_object ( builder, "image" );
    GObject *tipLabel = gtk_builder_get_object ( builder, "tip_label" );

    swd->replaceButton = (GtkWidget*)replaceButton;
    swd->addButton = (GtkWidget*)addButton;
    swd->cancelButton = (GtkWidget*)cancelButton;
    swd->setButton = (GtkWidget*)setButton;
    swd->removeButton = (GtkWidget*)removeButton;
    swd->image = (GtkWidget*)image;
    swd->tipLabel = (GtkWidget*)tipLabel;

    gtk_widget_hide ( (GtkWidget*)replaceButton );
    gtk_widget_hide ( (GtkWidget*)addButton );
    gtk_widget_hide ( (GtkWidget*)cancelButton );
    gtk_widget_hide ( (GtkWidget*)setButton );
    gtk_widget_hide ( (GtkWidget*)removeButton );

    g_signal_connect ( (GtkWidget*)replaceButton, "clicked",
            G_CALLBACK(replace_button_click_cb), settingWindowData );

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ( 
            expanduser("/home/$USER/.stran/enter-keyboard-shortcut.svg"), NULL
            );
    gtk_image_set_from_pixbuf ( GTK_IMAGE(image), pixbuf );

    toggleWindowSensitive ( settingWindowData, FALSE );

    grab_seat ( settingWindowData );

    g_signal_connect ( GTK_CONTAINER(dialog), "destroy",
            G_CALLBACK(on_dialog_destroy_cb), settingWindowData );

    g_signal_connect ( GTK_WINDOW(dialog), "key-press-event", 
            G_CALLBACK(on_dialog_key_press_event_cb), settingWindowData );

    g_signal_connect ( replaceButton, "clicked",
            G_CALLBACK(replace_button_click_cb), settingWindowData );

    gtk_widget_show ( (GtkWidget*)dialog );
    gtk_widget_grab_focus ( (GtkWidget*) dialog );
}

void addShortcutToListBox ( 
        char (*shortcut)[SHORTCUT_CONTENT_LEN],
        char (*key)[SHORTCUT_CONTENT_LEN],
        SettingWindowData *settingWindowData ) {

    GtkListBox *list = (GtkListBox*)settingWindowData->shortcutSettingWindowData->listBox;
    GtkWidget *grid;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *button;

    /* grid->box->label grid->box->label*/
    for ( int i=0; i<MAX_SHORTCUT_NUM; i++ ) {

        if ( *shortcut[i] ) {

            grid = gtk_grid_new();
            gtk_widget_set_size_request ( grid, -1, LISTBOX_ROW_HEIGHT );

            /* 快捷键名称*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request ( box, 250, -1 );

            label = gtk_label_new ( key[i] );
            gtk_grid_attach ( GTK_GRID(grid), box, 0, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );
            gtk_widget_set_vexpand ( label, TRUE );
            gtk_widget_set_hexpand ( label, TRUE );
            gtk_widget_set_margin_start ( label, MARGIN_START );
            gtk_widget_set_halign ( label, GTK_ALIGN_START );

            /* gtk_grid_attach ( GTK_GRID(grid), gtk_separator_new(GTK_ORIENTATION_VERTICAL), 1, 0, 1, 1 ); */

            /* 快捷键键值*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            gtk_widget_set_size_request ( box, 100, -1 );
            label = gtk_label_new ( shortcut[i] );
            gtk_widget_set_margin_end ( label, MARGIN_END );
            gtk_grid_attach ( GTK_GRID(grid), box, 1, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );
            gtk_widget_set_vexpand ( label, TRUE );
            gtk_widget_set_hexpand ( label, TRUE );
            gtk_widget_set_halign ( label, GTK_ALIGN_START );

            /* Clear 图标*/
            box = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
            gtk_widget_set_size_request ( box, 50, -1 );
            image = gtk_image_new_from_icon_name ( "edit-clear", 3 );
            button = gtk_button_new();
            gtk_button_set_image ( GTK_BUTTON(button), image );
            gtk_button_set_relief ( GTK_BUTTON(button), None  );
            gtk_grid_attach ( GTK_GRID(grid), box, 2, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), button, 1, 1, 1 );
            /* gtk_widget_set_vexpand ( button, TRUE ); */
            /* gtk_widget_set_hexpand ( button, TRUE ); */
            gtk_widget_set_halign ( button, GTK_ALIGN_CENTER );
            gtk_widget_set_valign ( button, GTK_ALIGN_CENTER );
            g_signal_connect ( button, "clicked", 
                    G_CALLBACK(on_clear_button_click_cb), settingWindowData);


            gtk_grid_set_column_spacing ( GTK_GRID(grid), 36 );

            gtk_list_box_insert ( GTK_LIST_BOX(list), grid, -1 );
        }
    }

    g_signal_connect ( list, "row-activated",
            G_CALLBACK(on_listbox_row_selected_cb), settingWindowData );

    gtk_widget_show_all ( settingWindowData->contentScrollWindow );
}

void shortcutSetting(  SettingWindowData *settingWindowData   ) {

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)settingWindowData->contentScrollWindow,
            remove_widget,
            (settingWindowData->contentScrollWindow)
            );

    static ShortcutSettingWindowData shortcutSettingWindowData;
    memset ( &shortcutSettingWindowData, '\0', sizeof(shortcutSettingWindowData) );
    settingWindowData->shortcutSettingWindowData = &shortcutSettingWindowData;

    /* GtkWidget *delayScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,0,2000, 1); */
    /* gtk_widget_set_size_request ( delayScale, 500, 500 ); */
    /* gtk_container_add ( GTK_CONTAINER(settingWindowData->contentScrollWindow), delayScale); */

    /* GTK BOX*/
    GtkWidget *box = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 ); 
    gtk_container_add ( GTK_CONTAINER(settingWindowData->contentScrollWindow), box );
    gtk_widget_set_hexpand ( box, TRUE  );
    gtk_widget_set_size_request ( settingWindowData->contentScrollWindow, 400, 400 );
    gtk_widget_set_margin_start(box, MARGIN_START );
    gtk_widget_set_margin_end(box, MARGIN_END );
    gtk_widget_set_margin_top(box, MARGIN_TOP );

    /* GTK LIST BOX */
    GtkWidget *listBox = gtk_list_box_new();
    settingWindowData->shortcutSettingWindowData->listBox = listBox;
    gtk_box_pack_start ( GTK_BOX(box), listBox, 0,0,0 );
    gtk_widget_set_halign ( listBox, GTK_ALIGN_CENTER );

    static char shortcut[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN];
    static char keyName[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN];
    memset ( shortcut, '\0', sizeof(shortcut) );

    readFromConfigByKeyword ( shortcut, "Shortcut" );
    readNameByKeyword ( keyName, "Shortcut" );

    addShortcutToListBox ( shortcut, keyName, settingWindowData ); 

    gtk_widget_show_all ( settingWindowData->contentScrollWindow );
}
