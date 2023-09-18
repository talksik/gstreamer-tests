#!/bin/bash

rm -rf mux.mp4

gst-launch-1.0 alsasrc ! voaacenc ! queue ! mux. \
    v4l2src ! video/x-raw,width=640,height=480,framerate=30/1 ! \
    x264enc ! h264parse ! \
    mp4mux name=mux ! \
    filesink location="mux.mp4"
