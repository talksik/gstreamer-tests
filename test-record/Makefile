
run: clean build
	./main

build:
	gcc -o main main.c `pkg-config --cflags --libs gstreamer-1.0 glib-2.0`

clean:
	rm -f myprog
	rm -f *.mp4
