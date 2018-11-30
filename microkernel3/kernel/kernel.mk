ifneq ($(LAYER),)
override CFLAGS += -DLAYER="$(LAYER)"
LAYER_DIR := layers/$(LAYER)
include $(LAYER_DIR)/kext/kext.mk
endif

objs := init.o memory.o process.o thread.o ipc.o timer.o server.o printk.o string.o list.o kfs.o

ARCH_DIR = kernel/arch/$(ARCH)
KFS_DIR = $(BUILD_DIR)/kernel/kfs
include VERSION
include $(ARCH_DIR)/arch.mk

ifeq ($(KERNEL_TEST),1)
override CFLAGS += -DKERNEL_TEST
objs := init.o memory.o process.o thread.o ipc.o printk.o string.o list.o test.o timer.o
endif

objs := $(addprefix $(BUILD_DIR)/kernel/, $(objs)) \
	$(addprefix $(BUILD_DIR)/$(ARCH_DIR)/, $(arch_objs)) \
	$(addprefix $(BUILD_DIR)/$(LAYER_DIR)/kext/, $(kext_objs))
kfs_files := $(addprefix $(KFS_DIR)/servers/, $(SERVERS))

# Load libs.
include_dirs := $(PWD) $(addprefix $(ARCH_DIR)/, $(arch_include_dirs)) $(BUILD_DIR)/stubs/c

$(BUILD_DIR)/kernel/kernel.elf: $(objs) $(kernel_ld)
	$(PROGRESS) "LD" $@
	$(LD) $(LDFLAGS) $(KERNEL_LDFLAGS) -o $@ $(objs)

$(BUILD_DIR)/%.o: %.S Makefile
	mkdir -p $(dir $@)
	$(PROGRESS) "CC" $@
	$(CC) $(CFLAGS) $(addprefix -I, $(include_dirs)) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(PROGRESS) "CC" $@
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' $(addprefix -I, $(include_dirs)) -c -o $@ $<
	$(CC) $(CFLAGS) $(addprefix -I, $(include_dirs)) -MF $(@:.o=.deps) -MT $(BUILD_DIR)/$(<:.c=.o) -MM $<

# KFS
$(BUILD_DIR)/kernel/kfs.o: $(BUILD_DIR)/kernel/kfs.bin

$(BUILD_DIR)/kernel/kfs.bin: $(kfs_files) tools/mkkfs.py
	$(PROGRESS) MKKFS $@
	./tools/mkkfs.py $@ $(KFS_DIR)

include $(wildcard $(objs:.o=.deps))
