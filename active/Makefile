
CFLAGS +=-g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings
GENERAL_LIBRARIES := -lm -lpthread
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt

all: main test_freetype

main.o : main.c
	gcc -c $^ ${CFLAGS}

gl3w.o : gl3w.c
	gcc -c $^ ${CFLAGS}

main : main.o gl3w.o
	gcc $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES}

test_freetype : test_freetype.o gl3w.o
	gcc $^ -o $@ ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES}

clean:
	@rm -rf *.o
	@rm -rf vgcore*
