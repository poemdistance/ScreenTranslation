#include <gtk/gtk.h>
#include <stdlib.h>
#include "common.h"
#include "quickSearch.h"

typedef struct Widget {

    char *buf;
    GtkWidget *window;
    GtkWidget *entry;

}Widget;

void search_change ( GtkSearchEntry *entry, gpointer *data ) {


    const char *buf;
    buf = gtk_entry_get_text ( (GtkEntry*)entry );

    strcpy ( (char*)data, buf );
}

void stop_search (  GtkSearchEntry *entry, gpointer *data  ) {

    char *shmaddr;
    shared_memory_for_keyboard_event(&shmaddr);

    /* 退出快捷键标志位清零*/
    shmaddr[1] = '0';

    /* quick search 快捷键标志位清零*/
    shmaddr[0] = '0';

    gtk_widget_destroy ( (GtkWidget*)data );
    gtk_widget_destroy ( (GtkWidget*)entry );
    gtk_main_quit();
}

void submit_text ( GtkSearchEntry *entry, gpointer *data ) {

    Widget *wg = (Widget*)data;

    if ( isEmpty( wg->buf ) )
        return;

    strcpy ( wg->buf, \
            gtk_entry_get_text ( (GtkEntry*)entry ));
    g_print ("Get text: %s\n", wg->buf);

    char *shmaddr = NULL;
    shared_memory_for_quickSearch( &shmaddr );

    /* 文本提交标记位*/
    shmaddr[TEXT_SUBMIT_BYTE] = '1';

    strcat ( wg->buf, "\n" );

    /* 注意结尾字符, 应该是有的*/
    strcpy ( &shmaddr[SUBMIT_TEXT], wg->buf);

    gtk_widget_destroy ( wg->entry );
    gtk_widget_destroy ( wg->window );
    gtk_main_quit();
}


/* 键值测试代码回调函数*/
void key_event ( GtkSearchEntry *entry, GdkEventKey *event, gpointer data ) {

    /* 打印出键值*/
    g_print ("%d\n", event->keyval);
}

void searchWindow() {

    GtkWidget *window;

    char buf[1024] =  { '\0' };

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *search = gtk_search_entry_new ();
    gtk_widget_grab_focus ( window );
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    gtk_window_set_keep_above ( GTK_WINDOW(window), TRUE );
    gtk_window_set_position ( GTK_WINDOW(window), GTK_WIN_POS_CENTER );
    gtk_window_set_decorated ( GTK_WINDOW(window), FALSE );
    gtk_window_set_title(GTK_WINDOW(window), "");
    gtk_container_add ( GTK_CONTAINER(window), search );
    gtk_entry_set_placeholder_text ( (GtkEntry*)search, "Enter to submit, Ctrl-c to exit" );

    /* 获取Class*/
    GtkEntryClass *klass = GTK_ENTRY_GET_CLASS ( (GtkEntry*)search );

    /* 等价GObjectClass *class = (GObjectClass*)( klass ); */
    GObjectClass *class = G_OBJECT_CLASS ( klass );
    GtkBindingSet *bset = gtk_binding_set_by_class ( class );


    /* 绑定退出键为Ctrl-C*/
    gtk_binding_entry_add_signal ( bset, GDK_KEY_c, GDK_CONTROL_MASK, "stop-search", 0 );

    /* GDK_KEY_Return 等价于整数:65293*/
    gtk_binding_entry_add_signal ( bset, GDK_KEY_Return, 0, "next-match", 0 );

    g_signal_connect(G_OBJECT(window), "destroy", \
            G_CALLBACK(gtk_main_quit),NULL);

    g_signal_connect ( G_OBJECT( search ), "search-changed",\
            G_CALLBACK( search_change ), buf);

    g_signal_connect ( G_OBJECT( search ), "stop-search",\
            G_CALLBACK( stop_search ), window);

    /* 键值测试代码*/
    //g_signal_connect ( G_OBJECT( search ), "key-press-event", G_CALLBACK(key_event), NULL );

    Widget wg;
    wg.buf = buf;
    wg.window = window;
    wg.entry = search;
    g_signal_connect ( G_OBJECT ( search ), "next-match", G_CALLBACK( submit_text ), &wg );

    gtk_window_activate_focus ( (GtkWindow*)window );
    gtk_widget_show_all(window);

    gtk_main();
}

