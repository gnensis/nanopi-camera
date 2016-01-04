CC = gcc
CFLAGS = -Wall -Werror -Wcast-align -g
LDFLAGS =
 
SWCOBJECT = swc.o opt.o video.o screen.o
 
all: swc
 
swc: $(SWCOBJECT)
	$(CC) $(LDFLAGS) `pkg-config --libs sdl` $(SWCOBJECT) -o $@
 
swc.o: swc.c
	$(CC) $(CFLAGS) `pkg-config --cflags sdl` -c $<
 
opt.o: opt.c opt.h
	$(CC) $(CFLAGS) -c $<
 
video.o: video.c video.h
	$(CC) $(CFLAGS) -c $<
 
screen.o: screen.c screen.h
	$(CC) $(CFLAGS) `pkg-config --cflags sdl` -c $<
 
clean:
	rm -f *.o *~ swc
 
.PHONY: all clean
