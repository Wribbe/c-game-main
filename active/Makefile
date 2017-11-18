
CFLAGS +=-g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings
GENERAL_LIBRARIES := -lm -lpthread
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt
FONT_FLAGS := -I/usr/include/freetype2 -I/usr/include/libpng16 \
			  -I/usr/include/harfbuzz -I/usr/include/glib-2.0  \
			  -I/usr/lib/glib-2.0/include

CC=gcc

all: main test_freetype 3D_flight collisions gen_c

main.o : main.c
	${CC} -c $^ ${CFLAGS}

gl3w.o : gl3w.c
	${CC} -c $^ ${CFLAGS}

main : main.o gl3w.o
	${CC} $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES}

test_freetype.o : test_freetype.c
	${CC} -c $^ ${CFLAGS} ${FONT_FLAGS}

test_freetype : test_freetype.o gl3w.o
	${CC} $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

3D_flight: 3D_flight.o gl3w.o
	${CC} $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

collisions: collisions.o gl3w.o
	${CC} $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

gen_c.c: gen_c.py
	./$^ > $@

gen_c: gen_c.o gl3w.o
	${CC} $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES} ${FONT_FLAGS}

clean:
	@rm -rf *.o
	@rm -rf vgcore*
