ARCH_DIR = kernel/arch/$(ARCH)
include $(ARCH_DIR)/arch.mk

objs := init.o memory.o process.o thread.o ipc.o kfs.o elf.o server.o printk.o string.o list.o
stubs := discovery exit logging

kernel_objs := $(addprefix kernel/, $(objs)) $(addprefix $(ARCH_DIR)/, $(arch_objs))
kernel_libs := $(libs)
stub_files :=  $(foreach stub, $(stubs), build/resea/$(stub).h)

# Load libs.
all_objs :=
all_libs :=
all_include_dirs :=
included_subdirs :=
include $(foreach lib, $(kernel_libs), libs/$(lib)/build.mk)
kernel_objs := $(addprefix $(BUILD_DIR)/, $(kernel_objs) $(all_objs))
kernel_include_dirs := $(PWD) $(addprefix $(ARCH_DIR)/, $(arch_include_dirs)) \
	build $(all_include_dirs)

$(BUILD_DIR)/kernel/kfs.o: $(BUILD_DIR)/kernel/kfs.bin
$(BUILD_DIR)/kernel/kfs.bin: $(all_kfs_files) tools/mkkfs
	$(PROGRESS) MKKFS $@
	./tools/mkkfs $@ $(KFS_DIR)

$(BUILD_DIR)/kernel/kernel.elf: $(kernel_objs) $(ARCH_DIR)/kernel.ld
	$(PROGRESS) "LD(K)" $@
	$(LD) $(LDFLAGS) --Map=$(BUILD_DIR)/kernel/kernel.map --script $(ARCH_DIR)/kernel.ld -o $@ $(kernel_objs)

$(BUILD_DIR)/resea/%.h: idl/%.idl tools/genstub.py tools/idl/parser/idlParser.py
	mkdir -p $(dir $@)
	$(PROGRESS) GENSTUB $@
	./tools/genstub.py -o $(dir $@) $<

$(BUILD_DIR)/%.o: %.S Makefile
	mkdir -p $(dir $@)
	$(PROGRESS) "CC(K)" $@
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile $(stub_files)
	mkdir -p $(dir $@)
	$(PROGRESS) "CC(K)" $@
	$(CC) $(CFLAGS) $(addprefix -I, $(kernel_include_dirs)) -c -o $@ $<

$(BUILD_DIR)/%.deps: %.c Makefile $(stub_files)
	mkdir -p $(dir $@)
	$(PROGRESS) "GENDEPS(K)" $@
	$(CC) $(CFLAGS) $(addprefix -I, $(kernel_include_dirs)) -MF $@ -MT $(<:.c=.o) -MM $<

# Clear variables.
objs :=
libs :=
stub_files :=
include_dirs :=
subdirs :=
included_subdirs :=

-include $(kernel_objs:.o=.deps)
