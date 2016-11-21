# Gather data.
c_files := $(subst ./,, $(shell find . -name "*.c"))
c_folders := $(dir $(c_files))
includes := $(wildcard include/*)

# Construct all filenames.
c_names := $(notdir $(c_files))
o_names := $(c_names:.c=.o)
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

# Look for objects in $(dir_obj).
vpath %.o $(dir_obj)

# Look for *.c files in $(c_folders).
vpath %.c $(c_folders)

# Construct artifact paths.
executables := $(foreach filename,$(exec_names),$(dir_exec)/$(filename))

# Set up final flags.
my_flags = -Wall -Wextra -pedantic -std=gnu11 -Wwrite-strings -g -lm
audio_flags = -lportaudio -lasound -ljack
FLAGS = $(include_flags) $(audio_flags) $(my_flags) $(CFLAGS)

# Phony declarations.
.PHONY: all clean

all: $(executables)

clean:
	rm -rf $(dir_obj)
	rm -rf $(dir_exec)

%.o : %.c | mkdirs
	$(CC) -c $(FLAGS) $< -o $(dir_obj)/$@

$(dir_exec)/% : %.o
	$(CC) $(FLAGS) $(dir_obj)/$< -o $@

mkdirs:
	$(call conditional_mkdir,$(dir_exec))
	$(call conditional_mkdir,$(dir_obj))
