#include <gst/gst.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// gst-launch-1.0 v4l2src ! x264enc ! mp4mux ! filesink
// location=/home/rish/Desktop/okay.264 -e
static GMainLoop *loop;
static GstElement *pipeline, *src, *convert, *encoder, *muxer, *sink, *video_queue;
static GstElement *alsa_src, *audio_queue, *audio_convert, *audio_resample,
    *audio_encoder, *audio_parse;
static GstBus *bus;

static gboolean message_cb(GstBus *bus, GstMessage *message,
                           gpointer user_data) {
  switch (GST_MESSAGE_TYPE(message)) {
  case GST_MESSAGE_ERROR: {
    GError *err = NULL;
    gchar *name, *debug = NULL;

    name = gst_object_get_path_string(message->src);
    gst_message_parse_error(message, &err, &debug);

    g_printerr("ERROR: from element %s: %s\n", name, err->message);
    if (debug != NULL)
      g_printerr("Additional debug info:\n%s\n", debug);

    g_error_free(err);
    g_free(debug);
    g_free(name);

    g_main_loop_quit(loop);
    break;
  }
  case GST_MESSAGE_WARNING: {
    GError *err = NULL;
    gchar *name, *debug = NULL;

    name = gst_object_get_path_string(message->src);
    gst_message_parse_warning(message, &err, &debug);

    g_printerr("ERROR: from element %s: %s\n", name, err->message);
    if (debug != NULL)
      g_printerr("Additional debug info:\n%s\n", debug);

    g_error_free(err);
    g_free(debug);
    g_free(name);
    break;
  }
  case GST_MESSAGE_EOS: {
    g_print("Got EOS\n");
    g_main_loop_quit(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_main_loop_unref(loop);
    gst_object_unref(pipeline);
    exit(0);
    break;
  }
  default:
    break;
  }

  return TRUE;
}

int sigintHandler(int unused) {
  g_print("You ctrl-c-ed! Sending EoS");
  gst_element_send_event(pipeline, gst_event_new_eos());
  return 0;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, sigintHandler);
  gst_init(&argc, &argv);

  pipeline = gst_pipeline_new(NULL);
  src = gst_element_factory_make("v4l2src", NULL);
  convert = gst_element_factory_make("videoconvert", NULL);
  video_queue = gst_element_factory_make("queue", NULL);
  encoder = gst_element_factory_make("vaapih264enc", NULL);

  muxer = gst_element_factory_make("mp4mux", NULL);
  sink = gst_element_factory_make("filesink", NULL);

  alsa_src = gst_element_factory_make("alsasrc", NULL);
  audio_queue = gst_element_factory_make("queue", NULL);
  audio_convert = gst_element_factory_make("audioconvert", NULL);
  audio_resample = gst_element_factory_make("audioresample", NULL);
  audio_encoder = gst_element_factory_make("voaacenc", NULL);
  audio_parse = gst_element_factory_make("aacparse", NULL);

  if (!pipeline || !src || !convert || !encoder || !muxer || !sink) {
    g_error("Failed to create video elements");
    return -1;
  }

  if (!alsa_src || !audio_queue || !audio_convert || !audio_resample ||
      !audio_encoder || !audio_parse) {
    g_error("Failed to create audio elements");
    return -1;
  }

  g_object_set(sink, "location", "./out.mp4", "sync", false, NULL);
  gst_bin_add_many(GST_BIN(pipeline),
      src,
      video_queue,
      convert,
      encoder,
      muxer,
      sink,
      alsa_src,
      audio_queue,
      audio_convert,
      audio_resample,
      audio_encoder,
      audio_parse,
      NULL);
  if (!gst_element_link_many(src, convert, encoder, muxer, sink, NULL)) {
    g_error("Failed to link video elements");
    return -2;
  }

  if (!gst_element_link_many(alsa_src, audio_queue, audio_convert,
                             audio_resample, audio_encoder, audio_parse, muxer,
                             NULL)) {
    g_error("Failed to link audio elements");
    return -2;
  }

  loop = g_main_loop_new(NULL, FALSE);

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_add_signal_watch(bus);
  g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(message_cb), NULL);
  gst_object_unref(GST_OBJECT(bus));

  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  g_print("Starting loop");
  g_main_loop_run(loop);

  return 0;
}
