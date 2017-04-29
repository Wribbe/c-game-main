platform := Platform
extension := Extension
compiler := Compiler

headings :=		$(platform)		$(extension)	$(compiler)
platforms := 	win64			.exe 			x86_64-w64-mingw32-gcc 	\
				win32			.exe 			i686-w64-mingw32-gcc 	\
			 	linux 			.				gcc						\

directories := sources bin objects
keep := sources

SOURCE_EXTENSION := .c

$(foreach dir,$(directories),$(eval dir_$(dir) := $(dir)) $(call create_path,$(dir)))

SOURCE_NAMES := $(foreach path,$(wildcard $(dir_sources)/*$(SOURCE_EXTENSION)),$(notdir $(path)))

null :=
space := $(null) $(null)
space_sub := !!

create_path = $(if $(wildcard $(1)/*),,$(shell mkdir -p $(1)))

index_call =\
$(if $(filter $(1),$(firstword $(2))),\
	$(words x $(value 3)),\
	$(call index_call,$(1),$(filter-out $(firstword $(2)),$(2)),$(value 3) x)\
)
index = $(call index_call,$(1),$(headings))

pack = $(subst $(space),$(space_sub),$(foreach word,$(1),$(strip $(word))))
unpack = $(subst $(space_sub),$(space),$(1))

list_indexes =\
$(if $(filter $(words $(1) $(headings)),$(words $(2))),\
	$(value 3),\
	$(call list_indexes,$(1),\
		$(2) $(headings),\
		$(value 3) $(call pack,$(words $(value 4) x) $(words $(2))),\
		$(2)\
	)\
)

get = $(word $(call index,$(1)),$(call unpack,$(2)))

first = $(firstword $(call unpack,$(1)))
second = $(lastword $(call unpack,$(1)))

platform_strings :=\
$(foreach tuple,$(call list_indexes,$(platforms),$(headings)),\
	$(call pack,$(wordlist 	$(call first,$(tuple)),\
							$(call second,$(tuple)),\
							$(platforms)))\
)

strip_leading_dots = $(subst $(space),.,$(strip $(subst .,$(space),$(1))))
make_binmask = $(if $(call strip_leading_dots,$(1)),$(2)%$(1),$(2)%)

replace_extension =\
$(eval suff := $(call strip_leading_dots,$(call get,$(extension),$(2)))) \
$(if $(suff),$(1:$(SOURCE_EXTENSION)=.$(suff)),$(1:$(SOURCE_EXTENSION)=))

make_execs =\
$(foreach file,$(SOURCE_NAMES),\
	$(eval eval_dir := $(dir_bin)/$(call get,$(platform),$(1))) \
	$(eval eval_exec := $(call replace_extension,$(file),$(1))) \
	$(eval_dir)/$(eval_exec)\
)

extract_platforms = $(foreach string,$(1),$(call get,$(platform),$(string)))

all: $(call extract_platforms,$(platform_strings))

clean:
	@rm -rf $(filter-out $(keep),$(directories))

$(foreach string,$(platform_strings),\
	$(eval eval_platform := $(call get,$(platform),$(string))) \
	$(eval eval_dependencies := $(call make_execs,$(string))) \
	$(eval $(eval_platform): $(eval_dependencies) ;\
		@echo Compilation for platform $(eval_platform) done.)\
	$(eval eval_binpath := $(dir $(firstword $(eval_dependencies)))) \
	$(call create_path, $(eval_binpath)) \
	$(eval eval_extension := $(call get,$(extension),$(string))) \
	$(eval eval_exec_mask := $(call make_binmask,$(eval_extension),$(eval_binpath))) \
	$(eval eval_obj_path := $(dir_objects)/$(eval_platform)/$(notdir $(eval_exec_mask:$(eval_extension)=).o)) \
	$(call create_path, $(dir $(eval_obj_path))) \
	$(eval eval_compiler := $(call get,$(compiler),$(string))) \
	$(eval $(eval_obj_path) : $(dir_sources)/%.c ; \
		$(eval_compiler) $$^ -c -g -o $$@)  \
	$(eval $(eval_exec_mask) : $(eval_obj_path);\
		$(eval_compiler) -o $$@ $$^) \
)

.PHONY: all clean
