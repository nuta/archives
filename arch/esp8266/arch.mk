arch_objs += \
    panic.o      \
    boot.o       \
    init.o       \
    mutex.o      \
    printchar.o  \
    interrupt.o  \
    asm_thread.o \
    thread.o

CC := xtensa-lx106-elf-gcc
CXX := xtensa-lx106-elf-g++
LD := xtensa-lx106-elf-ld
STRIP := xtensa-lx106-elf-strip

COMMON += -ggdb3 -mlongcalls -mtext-section-literals -falign-functions=4
COMMON += -ffunction-sections -fdata-sections
CFLAGS += $(COMMON)
CXXFLAGS += $(COMMON)
CPPFLAGS += -I$(arch_dir)
LDFLAGS += -nostdlib -static -T$(arch_dir)/esp8266.lds -Map $(BUILD_DIR)/$(target).map

STRIPFLAGS += -s -R .comment -R .xtensa.info -R .xt.lit -R .xt.prop

$(TARGET_FILE): $(TARGET_FILE).debug
	$(CMDECHO) STRIP $@
	$(STRIP) $(STRIPFLAGS) $< -o $@

$(TARGET_FILE).debug: $(objs) $(arch_dir)/esp8266.lds
	$(CMDECHO) LD $@
	$(LD) $(LDFLAGS) -o $@ $(objs) $(shell $(CC) -print-libgcc-file-name)

