mkfiles := $(filter-out %/lib.mk, $(MAKEFILE_LIST))
build_mk := $(word $(words $(mkfiles)), $(mkfiles))
current_dir := $(dir $(build_mk))

ifneq ($(filter $(current_dir),$(included_subdirs)),$(current_dir))

all_objs += $(addprefix $(current_dir), $(objs))
all_libs += $(libs)
all_include_dirs += $(addprefix $(current_dir), $(include_dirs))
included_subdirs += $(current_dir)

endif

objs :=
libs :=
include_dirs :=
subdirs :=
