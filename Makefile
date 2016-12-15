# Gather data.
c_files := $(subst ./,,$(shell find -name "*.c" -not -path "./dependencies/*"))
c_folders := $(dir $(c_files))
include_folders := $(foreach path,$(wildcard include/*),$(patsubst %.h,,$(path)))
includes := include dependencies $(include_folders)


# Construct all filenames.
c_names := $(notdir $(c_files))
dep_source := $(foreach dep_source,\
				$(wildcard dependencies/*.c), \
				$(notdir $(dep_source)))
dep_obj := $(dep_source:.c=.o)
exec_names := $(c_names:.c=)
local_libs := $(wildcard libs/*.a)

# Add prefix function.
depp = $(addprefix $(dir_obj)/,$(1))

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
.PHONY: all clean vgclean

all: $(executables)

vgclean:
	rm -rf vgcore*

clean: vgclean
	rm -rf $(dir_obj)
	rm -rf $(dir_exec)

# Special case for glad.o compilation, can't be pedantic.
$(dir_obj)/glad.o: glad.c
	$(CC) -c $^ $(include_flags) -o $(dir_obj)/glad.o

# Boing and the remade example needs glad.o object linked.
$(dir_exec)/boing $(dir_exec)/redone_boing : $(dir_obj)/glad.o

# Link all objects with boing_dep.
$(dir_exec)/redone_boing : $(call depp,$(dep_obj))

$(dir_obj)/%.o : %.c | mkdirs
	@$(CC) $^ -c $(FLAGS) -o $@

#$(dir_exec)/% : $(dir_obj)/%.o
#	$(CC) $^ $(FLAGS) -o $@

#$(dir_exec)/% : $(call depp,$(global.o dep_obj)) $(dir_obj)/%.o
$(dir_exec)/% : $(dir_obj)/globals.o $(call depp,$(dep_obj)) $(dir_obj)/%.o
	@$(CC) -o $@ $^ $(FLAGS) $(local_libs)

mkdirs:
	$(call conditional_mkdir,$(dir_exec))
	$(call conditional_mkdir,$(dir_obj))
