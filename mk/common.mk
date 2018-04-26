mkfiles := $(filter-out %/common.mk, $(MAKEFILE_LIST))
build_mk := $(word $(words $(mkfiles)), $(mkfiles))
current_dir := $(dir $(build_mk))

all_objs += $(addprefix $(current_dir), $(objs))
all_libs += $(libs)
all_include_dirs += $(addprefix $(current_dir), $(include_dirs))

objs :=
libs :=
include_dirs :=
