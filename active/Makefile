# Makefile for C game project.
# ----------------------------

source_extension := c
dir_make_utils := ../makefile_utils
include $(dir_make_utils)/preamble_makefile

# Suppression control:
OUT_INFO := $(True)
OUT_WARNING := $(True)
#OUT_COMPILATION:=$(True)
OUT_COMPILATION := $(True)
OUTPUT_DYN_LIB := $(False)
OUTPUT_LIB := $(False)

PACMAN := $(shell command -v pacman 2> /dev/null)
ifndef PACMAN
$(error "pacman is not installed, aborting.")
endif


# Functionality toggles:
GIT_TARGETS := $(True)
SUPPRESS_GIT_OUTPUT := $(True)

# Compiler options:
# -----------------
CFLAGS += -g -std=c11 -Wall -Wextra -pedantic -Wwrite-strings
INCLUDE_FLAGS = -I$(dir_includes) -I$(dir_portaudio_source)/include \
				-I$(dir_flac_source)/include \
				-I$(dir_vorbisfile_source)/include/vorbis \
				-I$(dir_freetype2_source)/include
CFLAGS += $(INCLUDE_FLAGS)
DEPFLAGS = -MT $@ -MMD -MP -MF $(dir_deps)/$*.d
CXXFLAGS += $(DEPFLAGS)
CXX = gcc
LDFLAGS += #-L./$(dir_static_lib)
DYNAMIC_LDFLAGS += -L./$(dir_dynamic_lib)

GENERAL_LIBRARIES := -lm -lpthread
AUDIO_FLAGS := -lportaudio -lasound -ljack -logg -lvorbis
GRAPHICS_FLAGS := -lGLEW -lglfw3 -lGL -lX11 -lXrandr -lXi -lXxf86vm \
				  -ldl -lXinerama -lXcursor -lrt
FONT_FLAGS := -lpng -lz -lharfbuzz -lbz2

CFLAGS +=  $(GENERAL_LIBRARIES) $(AUDIO_FLAGS) $(GRAPHICS_FLAGS) $(FONT_FLAGS)

# Compilation commands:
o_from_cc = $(CXX) -c $< $(CFLAGS) $(CXXFLAGS) -o $@
exec_from_o = $(CXX) -o $@ $(call get_obj,$^) $(CFLAGS) $(call get_slibs,$^) \
			  $(call get_dlibs,$^)
a_from_os = ar crv $@ $^
so_from_os = $(CXX) -shared -Wl,-soname,$(call soname,$(notdir $@)) -o $@ $^

include $(dir_make_utils)/suppression_makefile

# Recipes.
# --------

# Library dependencies.

dir_exec := $(dir_exec_linux)
dir_objects := $(dir_objects_linux)
dir_static_lib := $(dir_static_lib_linux)
linux_execs = $(call execs, $(foreach source,$(CC_sources),\
			$(source:.$(source_extension)=))\
		  )

dir_exec := $(dir_exec_win)
dir_objects := $(dir_objects_win)
dir_static_lib := $(dir_static_lib_win)
linux_execs = $(call execs, $(foreach source,$(CC_sources),\
			$(source:.$(source_extension)=.exe))\
		  )

all: linux win | setup

set_linux_paths :

$(dir_exec_linux):
	mkdir $(dir_exec)

#linux : set_linux_paths $(linux_execs) | $(dir_exec_linux)
linux : set_linux_paths $(linux_execs)
	@echo $^

cross_compilation_win := /bin/x86_64-w64-mingw32-gcc
$(cross_compilation_win) :
	@echo "Could not find cross compilation toolkit, installing."
	sudo pacman -S mingw-w64-binutils mingw-w64-gcc

#win : $(cross_compilation_win) $(set_windows_paths) $(win_execs)
win :
	@echo "Hello from windows."


undescore := =======================

define generate_sh_runnner =
	rm -f $(2)
	echo "#!/bin/sh" > $(2)
	$(foreach exec,$(1),(echo "echo [~] Running $(exec)." &&\
						 echo "echo $(undescore)" &&\
						 echo "$(exec) || echo "[!] FAILED: $(exec)" && exit"&&\
						 echo "echo [~] $(exec) DONE."&&\
						 echo "echo ")\
						 >> $(2);)
	chmod +x $(2)
endef

$(linux_execs) : $(call slibs,libportaudio_static.a libpng.a libFLAC-static.a \
			libvorbisfile.a libfreetype.a)
$(linux_execs) : $(call objs,glad.o)

include $(dir_make_utils)/postamble_makefile
