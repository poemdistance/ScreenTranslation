#include "common.h"
#include <gst/gst.h>
#include <glib.h>

typedef struct Baidu {

    double width;
    double height;

    char *audio[2];

}Baidu;

int audioShow = -1;

extern Baidu bw;
extern char *baidu_result[BAIDUSIZE]; /* For Phonetic*/

    static gboolean
bus_call (GstBus     *bus,
        GstMessage *msg,
        gpointer    data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg)) {

        case GST_MESSAGE_EOS:
            g_print ("End of stream\n");
            g_main_loop_quit (loop);
            break;

        case GST_MESSAGE_ERROR: 
            {
                gchar  *debug;
                GError *error;

                gst_message_parse_error (msg, &error, &debug);
                g_free (debug);

                g_printerr ("Error: %s\n", error->message);
                g_error_free (error);

                g_main_loop_quit (loop);
                break;
            }
        default:
            break;
    }

    return TRUE;
}

int mp3play (GtkWidget *button, gpointer *data)
{
    audioShow = ~audioShow;
    char url[512] = { '\0' };

    if ( audioShow )
        strcpy ( url, bw.audio[0] );
    else
        strcpy ( url, bw.audio[1] );

    GMainLoop *loop;

    GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;
    GstBus *bus;
    guint bus_watch_id;

    /* Initialisation */
    gst_init (NULL, NULL);

    loop = g_main_loop_new (NULL, FALSE);

    /* Set up the pipeline */
    pipeline = gst_pipeline_new ("audio-player");

    /* This works well*/
    source   = gst_element_factory_make ("souphttpsrc",       "file-source");
    //source   = gst_element_factory_make ("filesrc",       "file-source");

    parser  = gst_element_factory_make ("mpegaudioparse", "mp3-parser");
    decoder  = gst_element_factory_make ("mpg123audiodec", "mp3-decoder");
    conv     = gst_element_factory_make ("audioconvert",  "converter");

    //sink     = gst_element_factory_make ("pulsesink", "audio-output");
    sink     = gst_element_factory_make ("autoaudiosink", "audio-output");

    if (!pipeline || !source || !parser || !decoder || !conv || !sink ) {
        g_printerr ("One element could not be created. Exiting.\n");
        return -1;
    }

    /*This work well with souphttpsrc, we set the input url to the source element */
    g_object_set (G_OBJECT (source), "location", url, NULL);

    /* we add a message handler */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    /* we add all elements into the pipeline */
    /* file-source | mp3-parser | mp3-decoder | converter | alsa-output */
    gst_bin_add_many (GST_BIN (pipeline),
            source, parser, decoder, conv, sink, NULL);

    /* we link the elements together */
    /* file-source -> mp3-parser ~> mp3-decoder -> converter -> alsa-output */
    gst_element_link_many (source, parser, decoder, conv, sink, NULL);


    /* Set the pipeline to "playing" state*/
    gst_element_set_state (pipeline, GST_STATE_PLAYING);


    /* Iterate */
    g_print ("Running...\n");
    g_main_loop_run (loop);


    /* Out of the main loop, clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));
    g_source_remove (bus_watch_id);
    g_main_loop_unref (loop);

    return 0;
}

GtkWidget* newVolumeBtn () 
{
    GtkWidget *button = gtk_button_new ( );
    GdkPixbuf *src = gdk_pixbuf_new_from_file ( "/home/rease/.stran/volume.png" , NULL);
    GdkPixbuf *dst = gdk_pixbuf_scale_simple ( src, 20, 20, GDK_INTERP_BILINEAR );

    GtkWidget *image = gtk_image_new_from_pixbuf ( dst );

    g_object_unref ( src );
    g_object_unref ( dst );

    gtk_button_set_image ( (GtkButton*)button, image );

    return button;
}

GtkWidget * insertVolumeIcon( GtkWidget *window, GtkLayout *layout ) 
{
    GtkWidget *button = newVolumeBtn();

    int charNum = countCharNums ( Phonetic );
    printf("\033[0;35mPhonetic charNum = %d \033[0m\n", charNum);

    gtk_layout_put ( layout, button, charNum * 12, 46 );

    g_signal_connect ( button, "clicked", G_CALLBACK(mp3play), NULL );

    printf("\033[0;34m重绘窗口 \033[0m\n");

    gtk_window_resize ( (GtkWindow*)window, bw.width-1, bw.height-1 );
    gtk_widget_queue_draw( window );

    printf("\033[0;34m重绘窗口完成 \033[0m\n");

    /* 设置透明度*/
    gtk_widget_set_opacity ( button, 0.7 );

    /* 一定要用这句, 不然新建的播放按钮不显示*/
    gtk_widget_show_all ( window );

    return button;
}
