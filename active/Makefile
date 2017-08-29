CFLAGS += -g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings
GENERAL_LIBRARIES := -lm -lpthread
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt

all: main

main : main.c
	gcc main.c -o main ${CFLAGS} ${GRAPHICS_FLAGS} ${GENERAL_LIBRARIES}
