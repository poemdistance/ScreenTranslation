#include "common.h"
#include <gst/gst.h>
#include <glib.h>
#include "newWindow.h"
#include "audio.h"
#include "expanduser.h"

extern char audioOnline_en[512];
extern char audioOnline_am[512];

extern char audioOffline_en[512];
extern char audioOffline_am[512];

char *getAudioSrcByButton ( GtkWidget *button, int *type, WinData *wd ) {

    static int source = -1;
    char *notEmptyEnSrc = NULL;
    char *notEmptyAmSrc = NULL;

    if ( strlen ( audioOffline_am )+strlen(audioOnline_en)
            && ( (*type=OFFLINE) || 1 ) )  {
        notEmptyAmSrc = audioOffline_am;
        notEmptyEnSrc = audioOffline_en;
    }
    else if ( strlen ( audioOnline_am ) + strlen(audioOnline_am)
            && ( (*type=ONLINE) || 1 )) {
        notEmptyAmSrc = audioOnline_am;
        notEmptyEnSrc = audioOnline_en;
    }


    if ( button == wd->audio_button_am ) {

        if ( wd->who == GOOGLE ) return notEmptyAmSrc;

        *type = ONLINE;
        if ( wd->who == BAIDU ) return audioOnline_am;

        *type = OFFLINE;
        if ( wd->who == MYSQL ) return audioOffline_am;
    }
    else if ( button == wd->audio_button_en ) {

        if ( wd->who == GOOGLE ) return notEmptyEnSrc;

        *type = ONLINE;
        if ( wd->who == BAIDU ) return audioOnline_en;

        *type = OFFLINE;
        if ( wd->who == MYSQL ) return audioOffline_en;
    }

    source = ~ source;

    if ( source ) 
        return notEmptyAmSrc;

    return notEmptyEnSrc;
}


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

gboolean mp3play (GtkWidget *button, WinData *wd)
{
    GMainLoop *loop;
    
    int type = 0;
    char *src = NULL;

    src = getAudioSrcByButton ( button, &type, wd );
    printf("Play Mp3: %s\n", src);

    GstElement *pipeline, *source, *parser, *decoder, *conv, *sink;
    GstBus *bus;
    guint bus_watch_id;

    /* Initialisation */
    gst_init (NULL, NULL);

    loop = g_main_loop_new (NULL, FALSE);

    /* Set up the pipeline */
    pipeline = gst_pipeline_new ("audio-player");

    if ( type == ONLINE )
        source   = gst_element_factory_make ("souphttpsrc",       "file-source");
    else if ( type == OFFLINE )
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
    g_object_set (G_OBJECT (source), "location", src, NULL);

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
