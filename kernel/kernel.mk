objs := init.o memory.o process.o thread.o ipc.o server.o printk.o string.o list.o
stubs := discovery exit logging io

ARCH_DIR = kernel/arch/$(ARCH)
KFS_DIR = $(BUILD_DIR)/kernel/kfs
C_STUB_DIR = $(BUILD_DIR)/stub/c
include VERSION
include $(ARCH_DIR)/arch.mk

kernel_objs := $(addprefix kernel/, $(objs)) $(addprefix $(ARCH_DIR)/, $(arch_objs))
libs := $(libs)
kfs_files := $(addprefix $(KFS_DIR)/servers/, $(SERVERS))

# Load libs.
all_objs :=
all_libs :=
all_include_dirs :=
included_subdirs :=
include $(foreach lib, $(libs), libs/$(lib)/build.mk)
objs := $(addprefix $(BUILD_DIR)/, $(kernel_objs) $(all_objs))
include_dirs := $(PWD) $(addprefix $(ARCH_DIR)/, $(arch_include_dirs)) \
	$(all_include_dirs) $(C_STUB_DIR)

$(BUILD_DIR)/kernel/kernel.elf: $(objs) $(kernel_ld)
	$(PROGRESS) "LD" $@
	$(LD) $(LDFLAGS) $(KERNEL_LDFLAGS) -o $@ $(objs)

$(BUILD_DIR)/%.o: %.S Makefile
	mkdir -p $(dir $@)
	$(PROGRESS) "CC" $@
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile $(C_STUB_DIR)
	mkdir -p $(dir $@)
	$(PROGRESS) "CC" $@
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' $(addprefix -I, $(include_dirs)) -c -o $@ $<
	$(CC) $(CFLAGS) $(addprefix -I, $(include_dirs)) -MF $(@:.o=.deps) -MT $(BUILD_DIR)/$(<:.c=.o) -MM $<

$(C_STUB_DIR): tools/idl/parser/idlParser.py tools/genstub.py $(foreach stub, $(stubs), interfaces/$(stub).idl)
	$(PROGRESS) "GENSTUB" $@
	./tools/genstub.py --idl-dir interfaces --out-dir $@ --lang c $(stubs)

# KFS
$(BUILD_DIR)/kernel/init.o: $(BUILD_DIR)/kernel/kfs.bin

$(BUILD_DIR)/kernel/kfs.bin: $(kfs_files) tools/mkkfs.py
	$(PROGRESS) MKKFS $@
	./tools/mkkfs.py $@ $(KFS_DIR)

include $(wildcard $(objs:.o=.deps))
