
# record mux another example with cli

```sh
# mux to file with fake audio + video
gst-launch-1.0 -e videotestsrc ! x264enc ! mp4mux name=mux ! filesink location="bla.mp4"  audiotestsrc ! lamemp3enc ! mux.
# diff mux than above
gst-launch-1.0 -vv videotestsrc ! videoconvert ! x264enc ! avimux name=mux ! filesink location="avimux_test.mp4"  audiotestsrc ! voaacenc ! mux.^C

# above but with actual audio src
gst-launch-1.0 videotestsrc ! x264enc ! mp4mux name=mux ! filesink location="bla.mp4"  pulsesrc ! lamemp3enc ! mux.

# and.... yep now with actual video src
# audio a little low?
gst-launch-1.0 -e v4l2src device="/dev/video0" ! videoconvert ! queue ! x264enc tune=zerolatency ! mux. pulsesrc ! queue ! audioconvert ! audioresample ! voaacenc ! aacparse ! qtmux name=mux ! filesink location=test_gstreamer_record.mp4 sync=false

# same thing but works fine after restart. make sure alsa settings not screwed up tho?
gst-launch-1.0 -e v4l2src device="/dev/video0" ! videoconvert ! queue ! x264enc tune=zerolatency ! mux. alsasrc ! queue ! audioconvert ! audioresample ! voaacenc ! aacparse ! qtmux name=mux ! filesink location=test.mp4 sync=false

# WITH vaapih264enc (hw acceleration if hardware + driver support on system)
# note: weird that we need h264parse maybe because of needed mux sink capabilities
gst-launch-1.0 -e v4l2src device="/dev/video0" ! videoconvert ! queue ! vaapih264enc ! h264parse ! mux. alsasrc ! queue ! audioconvert ! audioresample ! voaacenc ! aacparse ! qtmux name=mux ! filesink location=test.mp4 sync=false

```

# ffmpeg
```sh
# video record
ffmpeg -f v4l2 -framerate 25 -video_size 640_480 -i dev/video0 output.mkv

# video + audio record to file
ffmpeg -f alsa -i hw:0 -f video4linux2 -i /dev/video0

# audio only
ffmpeg -f alsa -i hw:0 output.mp3

# another one with encoder set and also using pulse
ffmpeg -f pulse -ac 2 -i default -f v4l2 -i /dev/video0 -t 00:00:20 -vcodec libx264 record.mp4

```
