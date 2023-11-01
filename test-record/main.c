#include <gst/gst.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

// gst-launch-1.0 v4l2src ! x264enc ! mp4mux ! filesink location=/home/rish/Desktop/okay.264 -e
static GMainLoop *loop;
static GstElement *pipeline, *src, *convert, *encoder, *muxer, *sink;
static GstBus *bus;

static gboolean
message_cb (GstBus * bus, GstMessage * message, gpointer user_data)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *name, *debug = NULL;

      name = gst_object_get_path_string (message->src);
      gst_message_parse_error (message, &err, &debug);

      g_printerr ("ERROR: from element %s: %s\n", name, err->message);
      if (debug != NULL)
        g_printerr ("Additional debug info:\n%s\n", debug);

      g_error_free (err);
      g_free (debug);
      g_free (name);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_WARNING:{
		GError *err = NULL;
		gchar *name, *debug = NULL;

		name = gst_object_get_path_string (message->src);
		gst_message_parse_warning (message, &err, &debug);

		g_printerr ("ERROR: from element %s: %s\n", name, err->message);
		if (debug != NULL)
		g_printerr ("Additional debug info:\n%s\n", debug);

		g_error_free (err);
		g_free (debug);
		g_free (name);
		break;
    }
    case GST_MESSAGE_EOS:{
		g_print ("Got EOS\n");
		g_main_loop_quit (loop);
		gst_element_set_state (pipeline, GST_STATE_NULL);
		g_main_loop_unref (loop);
		gst_object_unref (pipeline);
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

int main(int argc, char *argv[])
{
	signal(SIGINT, sigintHandler);
	gst_init (&argc, &argv);

	pipeline = gst_pipeline_new(NULL);
	src = gst_element_factory_make("v4l2src", NULL);
  convert = gst_element_factory_make("videoconvert", NULL);
	encoder = gst_element_factory_make("x264enc", NULL);
	muxer = gst_element_factory_make("mp4mux", NULL);
	sink = gst_element_factory_make("filesink", NULL);

	if (!pipeline || !src || !convert || !encoder || !muxer || !sink) {
		g_error("Failed to create elements");
		return -1;
	}

	g_object_set(sink, "location", "/home/talksik/code/gstreamer-tests/test-record/out.mp4", NULL);
	gst_bin_add_many(GST_BIN(pipeline), src, convert, encoder, muxer, sink, NULL);
	if (!gst_element_link_many(src, convert, encoder, muxer, sink, NULL)){
		g_error("Failed to link elements");
		return -2;
	}

	loop = g_main_loop_new(NULL, FALSE);

	bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(message_cb), NULL);
	gst_object_unref(GST_OBJECT(bus));

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	g_print("Starting loop");
	g_main_loop_run(loop);

	return 0;
}
