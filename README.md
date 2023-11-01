
# record mux another example with cli

```sh
gst-launch-1.0 -e videotestsrc ! x264enc ! mp4mux name=mux ! filesink location="bla.mp4"  audiotestsrc ! lamemp3enc ! mux.
```
