#!/bin/bash


rm -rf dump.ogg

gst-launch-1.0 pulsesrc ! audioconvert ! vorbisenc ! oggmux ! filesink location=dump.ogg

