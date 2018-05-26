include mk/common.mk
include $(DIR)/Makefile
server_build_dir := $(BUILD_DIR)/$(DIR)
executable := $(server_build_dir)/server.elf

.PHONY: build
build: $(executable)

ifeq ($(lang),)
$(error "$(DIR)/Makefile: lang is not set")
endif

# Rust projects.
ifeq ($(lang), rust)
abs_server_build_dir = $(PWD)/$(server_build_dir)
$(executable):
	mkdir -p $(server_build_dir)
	$(PROGRESS) GEN $(server_build_dir)/$(ARCH).json
	./libs/rust/gen-target-json.py $(ARCH) $(abs_server_build_dir)/$(ARCH).json

	$(PROGRESS) XARGO $(server_build_dir)
	cd $(DIR) && \
	CARGO_TARGET_DIR=$(abs_server_build_dir) \
	RUST_TARGET_PATH=$(abs_server_build_dir) \
		xargo build --target $(ARCH)
	cp $(server_build_dir)/$(ARCH)/debug/$(name) $@
endif

# C projects.
ifeq ($(lang), c)
# FIXME: define CFLAGS by ourselves
include kernel/arch/$(ARCH)/arch.mk

server_libs := resea $(filter-out resea, $(libs))
server_objs := $(foreach obj, $(objs), $(DIR)/$(obj))
server_include_dirs := $(include_dirs) $(BUILD_DIR)/stubs
name :=
objs :=
libs :=
include_dirs :=
all_objs :=
all_libs :=
all_include_dirs :=
included_subdirs :=
include $(foreach lib, $(server_libs), libs/$(lib)/build.mk)
server_include_dirs += $(server_build_dir) $(all_include_dirs)

objs := $(server_objs) $(all_objs)
stub_files := $(foreach stub, $(requires), $(BUILD_DIR)/stubs/resea/$(stub).h)
c_objs := $(addprefix $(server_build_dir)/, \
	$(patsubst %.c, %.o, $(wildcard $(objs:.o=.c))))
s_objs := $(addprefix $(server_build_dir)/, \
	$(patsubst %.S, %.o, $(wildcard $(objs:.o=.S))))

$(executable): $(c_objs) $(s_objs)
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script libs/resea/arch/$(ARCH)/app.ld -o $@ $^
	cp $@ $@.debug
	$(PROGRESS) STRIP $@
	$(STRIP) $@

$(c_objs): $(server_build_dir)/%.o: %.c $(stub_files)
	$(PROGRESS) CC $@
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I, $(server_include_dirs)) -c -o $@ $<

$(s_objs): $(server_build_dir)/%.o: %.S
	$(PROGRESS) CC $@
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I, $(server_include_dirs)) -c -o $@ $<
endif
