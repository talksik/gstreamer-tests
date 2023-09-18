#!/bin/bash

rm -rf test.mp4

export GST_DEBUG=3
gst-launch-1.0 -e v4l2src ! video/x-raw,width=640,height=480,framerate=30/1 ! \
  videoconvert ! \
  x264enc tune=zerolatency ! \
  mux. alsasrc ! queue ! audioconvert ! \
  audioresample ! voaacenc ! aacparse ! qtmux name=mux ! \
  filesink location=test.mp4 sync=false

