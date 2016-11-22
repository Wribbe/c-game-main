# Gather data.
c_files := $(subst ./,,$(shell find -name "*.c" -not -path "./dependencies/*"))
c_folders := $(dir $(c_files))
includes := include dependencies


# Construct all filenames.
c_names := $(notdir $(c_files))
exec_names := $(c_names:.c=)

# Set up flags.
include_flags := $(foreach i_flag,$(includes),-I$(i_flag))

# Directories setup.
dir_exec := executables
dir_obj := objects

conditional_mkdir = \
	@if [ ! -d $(1) ]; then \
		mkdir $(1); \
	fi

# Look for *.c files in $(c_folders) and dependencies.
vpath %.c $(c_folders) dependencies

# Construct artifact paths.
executables := $(foreach filename,$(exec_names),$(dir_exec)/$(filename))

# Set up final flags.
error_flags := -Wall -Wextra -pedantic -Wwrite-strings
compilation_options := -std=gnu11 -g
general_libraries := -lm -lpthread
audio_flags := -lportaudio -lasound -ljack
graphics_flags := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt
FLAGS = $(include_flags) $(compilation_options) $(graphics_flags) \
		$(audio_flags) $(error_flags) $(general_libraries) $(CFLAGS)

# Phony declarations.
.PHONY: all clean

all: $(executables)

clean:
	rm -rf $(dir_obj)
	rm -rf $(dir_exec)

# Special case for glad.o compilation, can't be pedantic.
$(dir_obj)/glad.o: glad.c
	$(CC) -c $^ $(include_flags) -o $(dir_obj)/glad.o

# Boing and the remade example needs glad.o object linked.
$(dir_exec)/boing $(dir_exec)/redone_boing : $(dir_obj)/glad.o

# Link events library for redo_boing.
$(dir_exec)/redone_boing : $(dir_obj)/events.o $(dir_obj)/graphics.o

$(dir_obj)/%.o : %.c | mkdirs
	$(CC) -c $(FLAGS) $^ -o $@

$(dir_exec)/% : $(dir_obj)/%.o
	$(CC) $^ $(FLAGS) -o $@

mkdirs:
	$(call conditional_mkdir,$(dir_exec))
	$(call conditional_mkdir,$(dir_obj))
