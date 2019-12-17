#include "common.h"
#include <gst/gst.h>
#include <glib.h>
#include "newWindow.h"
#include "audio.h"

#define status(who) ( ( who ) == BAIDU ? ( ONLINE ) : ( OFFLINE ) )
#define audio(type)  ( ( type ) == ONLINE ? ( url_online ) : ( url_offline ) )

/* 用于切换英音和美音*/
int audioShow = -1;

extern char *baidu_result[BAIDUSIZE]; /* For Phonetic*/
extern char *mysql_result[BAIDUSIZE]; /* For Phonetic*/

extern char audioOnline_en[512];
extern char audioOnline_uk[512];

extern char audioOffline_en[512];
extern char audioOffline_uk[512];

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

    int type = *(int *)data;

    audioShow = ~audioShow;
    char url_online[512] = { '\0' };
    char url_offline[512] = { '\0' };

    if ( audioShow ) {

        strcpy ( url_online, bw.audio_online[0] );
        strcpy ( url_offline, bw.audio_offline[0] );
    }
    else {

        strcpy ( url_online, bw.audio_online[1] );
        strcpy ( url_offline, bw.audio_offline[1] );
    }

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
    if ( type == BAIDU )
        source   = gst_element_factory_make ("souphttpsrc",       "file-source");
    else if ( type == OFFLINE )
        source   = gst_element_factory_make ("filesrc",       "file-source");
    else
        return -1;

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
    g_object_set (G_OBJECT (source), "location", audio(status(((WinData*)data)->who)), NULL);

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

GtkWidget * insertVolumeIcon( GtkWidget *window, GtkWidget *layout, WinData *wd, int type ) 
{
    GtkWidget *button = newVolumeBtn();

    int charNum = countCharNums ( Phonetic(type) );
    pbgreen("Phonetic charNum = %d \n", charNum);


    int posx = 0;
    int x = charNum;

    /* 三次函数拟合字符长度和按钮位置之间的关系*/
    posx = (int)(-9.33984e-06 * x*x*x  + 0.0567975 * x*x + 9.23914 *x + 40.5852 );

    pbgreen("pos.x=%d", posx);

    if ( charNum == 0 ) {
        pbred("charNum == 0, set posx to (wd->width)-rightBorderOffset");
        posx = wd->width - RIGHT_BORDER_OFFSET;
    }

    g_signal_connect ( button, "clicked", G_CALLBACK(mp3play), &wd->who );

    gtk_layout_put ( (GtkLayout*)layout, button, posx,  40 );
    gtk_widget_set_opacity ( button, 0.7 );

    /* 一定要用这句, 不然新建的播放按钮不显示*/
    gtk_widget_show_all ( window );

    return button;
}

void syncVolumeBtn ( WinData *wd, int type ) {

    /* 含音标，添加播放按钮*/
    if ( strlen ( Phonetic(type) ) != 0) {

        GtkWidget *volume =  ((WinData*)wd)->volume;

        if ( volume == NULL ) {

            bw.audio_online[0] = audio_en(ONLINE);
            bw.audio_online[1] = audio_uk(ONLINE);

            bw.audio_offline[0] = audio_en(OFFLINE);
            bw.audio_offline[1] = audio_uk(OFFLINE);

            ((WinData*)wd)->volume = insertVolumeIcon(((WinData*)wd)->window, ((WinData*)wd)->layout, ((WinData*)wd), type);
        }
        else {

            gtk_widget_show ( volume );
        }

    } 
    else {

        if ( WINDATA(wd)->volume )
            gtk_widget_hide ( WINDATA(wd)->volume   );
    }

}
