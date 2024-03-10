
#include <gst/gst.h>

gboolean on_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data) {
  GMainLoop *loop = (GMainLoop *)user_data;

  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    g_main_loop_quit(loop);
    break;
  case GST_MESSAGE_ERROR: {
    gchar *debug;
    GError *error;
    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);
    g_printerr("Error: %s\n", error->message);
    g_error_free(error);

    g_main_loop_quit(loop);
    break;
  }
  default:
    g_print("msg src %s, type: %d, name: %s\n", GST_MESSAGE_SRC_NAME(msg),
            GST_MESSAGE_TYPE(msg), GST_MESSAGE_TYPE_NAME(msg));
    break;
  }

  return TRUE;
}

void on_pad_added(GstElement *element, GstPad *pad, gpointer data) {
  GstElement *audio_dec = (GstElement *)data;

  /* We can now link this pad with the vorbis-decoder */
  g_print("Dynamic pad created, linking demuxer/decoder\n");

  GstPad *audio_sinkpad = gst_element_get_static_pad(audio_dec, "sink");
  if (gst_pad_can_link(pad, audio_sinkpad)) {
    gst_pad_link(pad, audio_sinkpad);
  }
  gst_object_unref(audio_sinkpad);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    g_printerr("Usage: %s <mp4 filename>\n", argv[0]);
    return -1;
  }

  // initialize gstreamer
  gst_init(&argc, &argv);

  // creat elements
  GstElement *pipeline = gst_pipeline_new("player");
  GstElement *source = gst_element_factory_make("filesrc", "source");
  GstElement *demuxer = gst_element_factory_make("qtdemux", "demuxer");
  GstElement *audio_dec = gst_element_factory_make("avdec_aac", "audio_dec");
  GstElement *audio_conv =
      gst_element_factory_make("audioconvert", "audio_conv");
  GstElement *audio_sink =
      gst_element_factory_make("autoaudiosink", "audio_sink");
  if (!pipeline || !source || !demuxer || !audio_dec || !audio_conv ||
      !audio_sink) {
    g_printerr("Not all elements could be created, pipeline 0x%p, source 0x%p, "
               "demuxer 0x%p, audio_dec 0x%p, audio_conv 0x%p, audio_sink "
               "0x%p\n",
               pipeline, source, demuxer, audio_dec, audio_conv, audio_sink);
    return -1;
  }

  // set file for playing
  g_object_set(G_OBJECT(source), "location", argv[1], NULL);

  // watch messages
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  guint bus_watch_id = gst_bus_add_watch(bus, on_bus_message, loop);
  gst_object_unref(bus);

  // connect
  gst_bin_add_many(GST_BIN(pipeline), source, demuxer, audio_dec, audio_conv,
                   audio_sink, NULL);
  gst_element_link(source, demuxer);
  gst_element_link_many(audio_dec, audio_conv, audio_sink, NULL);
  g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), audio_dec);

  // start to play
  g_print("Now playing: %s\n", argv[1]);
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  // loop
  g_main_loop_run(loop);

  g_print("loop finished, stopping");
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(pipeline));
  g_source_remove(bus_watch_id);
  g_main_loop_unref(loop);

  return 0;
}
