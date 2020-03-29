#include "common.h"
#include <gst/gst.h>
#include <glib.h>
#include "newWindow.h"
#include "audio.h"
#include "calibration.h"
#include "fitting.h"
#include "expanduser.h"

/* 用于切换英音和美音*/
int audioShow = -1;

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

gboolean mp3play (GtkWidget *button, gpointer *data)
{

    int type = *(int *)data;

    audioShow = ~audioShow;
    char *url_online = NULL;
    char *url_offline = NULL;

    if ( audioShow ) {

        url_online = bw.audio_online[0];
        url_offline = mw.audio_offline[0];
    }
    else {

        url_online = bw.audio_online[1];
        url_offline = mw.audio_offline[1];
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
    else if ( type == MYSQL )
        source   = gst_element_factory_make ("filesrc",       "file-source");
    else
        return FALSE;

    parser  = gst_element_factory_make ("mpegaudioparse", "mp3-parser");
    decoder  = gst_element_factory_make ("mpg123audiodec", "mp3-decoder");
    conv     = gst_element_factory_make ("audioconvert",  "converter");

    //sink     = gst_element_factory_make ("pulsesink", "audio-output");
    sink     = gst_element_factory_make ("autoaudiosink", "audio-output");

    if (!pipeline || !source || !parser || !decoder || !conv || !sink ) {
        g_printerr ("One element could not be created. Exiting.\n");
        return FALSE;
    }

    /*This work well with souphttpsrc, we set the input url to the source element */
    g_object_set (G_OBJECT (source), "location", AUDIO(TYPE(((WinData*)data)->who)), NULL);

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

    return TRUE;
}

void syncAudioBtn ( WinData *wd, int type ) {

    /* 含音标，添加播放按钮*/
    if ( strlen ( Phonetic(type) ) != 0) {

        GtkWidget *audio =  ((WinData*)wd)->audio;

        if ( audio == NULL ) {

            bw.audio_online[0] = AUDIO_EN(ONLINE);
            bw.audio_online[1] = AUDIO_UK(ONLINE);

            mw.audio_offline[0] = AUDIO_EN(OFFLINE);
            mw.audio_offline[1] = AUDIO_UK(OFFLINE);
        }
    } 
}
