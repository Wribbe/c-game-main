# Makefile for C++ laboratory session 02.
# ---------------------------------------

dir_make_utils = ../makefile_utils
include $(dir_make_utils)/preamble_makefile

# Output control:
export OUT_INFO := $(True)
export OUT_WARNING := $(True)
OUT_COMPILATION:=$(True)
#OUT_COMPILATION := $(False)
OUTPUT_DYN_LIB := $(False)
OUTPUT_LIB := $(False)


# Functionality toggles:
GIT_TARGETS := $(True)
SUPPRESS_GIT_OUTPUT := $(True)

# Compiler options:
# -----------------
CPPFLAGS += -g -std=c++11 -Wall -Wextra -pedantic-errors -Wold-style-cast
INCLUDE_FLAGS = -I$(dir_includes)
CPPFLAGS += $(INCLUDE_FLAGS)
DEPFLAGS = -MT $@ -MMD -MP -MF $(dir_deps)/$*.d
CXXFLAGS += $(DEPFLAGS)
CXX = g++
LDFLAGS += -L./$(dir_static_lib)
DYNAMIC_LDFLAGS += -L./$(dir_dynamic_lib)

# Compilation commands:
o_from_cc = $(CXX) -c $< $(CPPFLAGS) $(CXXFLAGS) -o $@
exec_from_o = $(CXX) -o $@ $(call get_obj,$^) $(CPPFLAGS) $(call get_slibs,$^) \
			  $(call get_dlibs,$^)
a_from_os = ar crv $@ $^
so_from_os = $(CXX) -shared -Wl,-soname,$(call soname,$(notdir $@)) -o $@ $^

include $(dir_make_utils)/suppression_makefile

# Recipes.
# --------

# Library dependencies.

# Exercise creation and running.

extxt = $(foreach exercise,${1},ex${exercise}.txt)
exercises = $(call extxt, A1)

all: $(exercises) | setup

$(call execs,nstest nstime): $(call objs,vns.o mns.o umns.o hns.o nameserverinterface.o)

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

$(dir_txt)/exA1.txt: $(call execs,nstest nstime)
	@$(call generate_sh_runnner,$^,$@)


include $(dir_make_utils)/postamble_makefile
