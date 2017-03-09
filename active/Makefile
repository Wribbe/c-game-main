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

CFLAGS +=  $(GENERAL_LIBRARIES) $(AUDIO_FLAGS) $(GRAPHICS_FLAGS)

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

execs := $(call execs, $(foreach source,$(CC_sources),\
			$(source:.$(source_extension)=))\
		  )

all: $(execs) | setup

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

$(execs) : $(call slibs,libportaudio_static.a libpng.a libFLAC-static.a \
			libvorbisfile.a libfreetype.a)
$(execs) : $(call objs,glad.o)

include $(dir_make_utils)/postamble_makefile
