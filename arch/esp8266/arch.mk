objs += \
    $(BUILD_DIR)/$(arch_dir)/ccount.o     \
    $(BUILD_DIR)/$(arch_dir)/panic.o      \
    $(BUILD_DIR)/$(arch_dir)/boot.o       \
    $(BUILD_DIR)/$(arch_dir)/init.o       \
    $(BUILD_DIR)/$(arch_dir)/mutex.o      \
    $(BUILD_DIR)/$(arch_dir)/printchar.o  \
    $(BUILD_DIR)/$(arch_dir)/halt.o       \
    $(BUILD_DIR)/$(arch_dir)/asm_thread.o \
    $(BUILD_DIR)/$(arch_dir)/thread.o

CC := xtensa-lx106-elf-gcc
CXX := xtensa-lx106-elf-g++
LD := xtensa-lx106-elf-gcc
STRIP := xtensa-lx106-elf-strip

COMMON += -g3 -ggdb3 -mlongcalls -mtext-section-literals -falign-functions=4
COMMON += -ffunction-sections -fdata-sections
COMMON += -DARCH_ESP8266
override CFLAGS   += $(COMMON)
override CXXFLAGS += $(COMMON)
override CPPFLAGS += -I$(arch_dir)
override LDFLAGS  += -nostdlib -static -Wl,-T$(arch_dir)/esp8266.lds -Wl,-Map,$(BUILD_DIR)/$(TARGET).map -flto

STRIPFLAGS += -s -R .comment -R .xtensa.info -R .xt.lit -R .xt.prop

$(TARGET_FILE): $(TARGET_FILE).debug
	$(CMDECHO) STRIP $@
	$(STRIP) $(STRIPFLAGS) $< -o $@

$(TARGET_FILE).debug: $(objs) $(arch_dir)/esp8266.lds
	$(CMDECHO) LD $@
	$(LD) $(LDFLAGS) -o $@ $(objs) $(shell $(CC) -print-libgcc-file-name)

