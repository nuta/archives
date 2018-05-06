mkfiles := $(filter-out %/server.mk, $(MAKEFILE_LIST))
build_mk := $(word $(words $(mkfiles)), $(mkfiles))
server_dir := $(patsubst %/,%, $(dir $(build_mk)))

libs += resea
requires += logging exit discovery benchmark

server_name := $(name)
server_build_dir := $(BUILD_DIR)/$(server_dir)
executable := $(server_build_dir)/server.elf
server_objs := $(foreach obj, $(objs), $(server_dir)/$(obj))
server_libs := $(libs)
server_include_dirs := $(include_dirs)

ifeq ($(lang), rust)
libs := rust $(filter-out rust, $(libs))
endif

# Load libs.
name :=
objs :=
libs :=
include_dirs :=
all_objs :=
all_libs :=
all_include_dirs :=
included_subdirs :=

# FIXME: refactor
include $(foreach lib, $(server_libs), libs/$(lib)/build.mk)
server_objs += $(all_objs)
server_include_dirs += $(server_build_dir) $(all_include_dirs)
server_stubs := $(foreach stub, $(requires), $(server_build_dir)/resea/$(stub).h)

server_c_objs := $(addprefix $(server_build_dir)/, \
	$(patsubst %.c, %.o, $(wildcard $(server_objs:.o=.c))))
server_s_objs := $(addprefix $(server_build_dir)/, \
	$(patsubst %.S, %.o, $(wildcard $(server_objs:.o=.S))))

$(server_build_dir)/resea/%.h: idl/%.idl tools/genstub/genstub.py tools/genstub/parser/idlParser.py
	mkdir -p $(dir $@)
	$(PROGRESS) GENSTUB $@
	./tools/genstub/genstub.py -o $(dir $@) $<


ifeq ($(lang), rust)
$(executable):
	cd $(server_dir) && \
	RUST_TARGET_PATH=$(PWD)/libs/rust/targets xargo rustc \
		--target $(ARCH) -- --emit obj=../../$@
else
$(executable): $(server_c_objs) $(server_s_objs)
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script libs/resea/arch/$(ARCH)/app.ld -o $@ $^
	cp $@ $@.debug
	$(PROGRESS) STRIP $@
	$(STRIP) $@
endif

$(info $(server_stubs))
$(server_c_objs): $(server_build_dir)/%.o: %.c $(server_stubs)
	$(PROGRESS) CC $@
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I, $(server_include_dirs)) -c -o $@ $<

$(server_s_objs): $(server_build_dir)/%.o: %.S
	$(PROGRESS) CC $@
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I, $(server_include_dirs)) -c -o $@ $<

# libs
$(server_build_dir)/%.o: %.c
	$(PROGRESS) CC $@
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I, $(server_include_dirs)) -c -o $@ $<

$(KFS_DIR)/servers/$(server_name): $(executable)
	$(PROGRESS) ADD $@
	mkdir -p $(dir $@)
	cp $< $@

# Add to kfs the executable.
ifeq ($(filter $(server_name),$(SERVERS)),$(server_name))
all_kfs_files += $(KFS_DIR)/servers/$(server_name)
endif

server_stubs :=
lang :=
all_objs :=
all_libs :=
all_include_dirs :=
included_subdirs :=
