
CFLAGS +=-g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings
GENERAL_LIBRARIES := -lm -lpthread
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt
FONT_FLAGS := -I/usr/include/freetype2 -I/usr/include/libpng16 \
			  -I/usr/include/harfbuzz -I/usr/include/glib-2.0  \
			  -I/usr/lib/glib-2.0/include

all: main test_freetype 3D_flight

main.o : main.c
	gcc -c $^ ${CFLAGS}

gl3w.o : gl3w.c
	gcc -c $^ ${CFLAGS}

main : main.o gl3w.o
	gcc $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES}

test_freetype.o : test_freetype.c
	gcc -c $^ ${CFLAGS} ${FONT_FLAGS}

test_freetype : test_freetype.o gl3w.o
	gcc $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

3D_flight: 3D_flight.o gl3w.o
	gcc $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

clean:
	@rm -rf *.o
	@rm -rf vgcore*
