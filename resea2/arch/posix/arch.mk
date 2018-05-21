objs += \
    $(BUILD_DIR)/$(arch_dir)/panic.o     \
    $(BUILD_DIR)/$(arch_dir)/boot.o      \
    $(BUILD_DIR)/$(arch_dir)/mutex.o     \
    $(BUILD_DIR)/$(arch_dir)/printchar.o \
    $(BUILD_DIR)/$(arch_dir)/halt.o      \
    $(BUILD_DIR)/$(arch_dir)/thread.o

LD := ld
override CFLAGS += -DARCH_POSIX -ggdb3 -Os
override CXXFLAGS += -DARCH_POSIX -ggdb3 -Os

ifeq ($(shell uname -s), Linux)
override CFLAGS += -pthread
endif

ifeq ($(shell uname -s), Darwin)
CC  := gcc-6
CXX := g++-6
endif

override CPPFLAGS += -I$(arch_dir)

$(TARGET_FILE): $(objs)
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -o $@ $(objs)

.PHONY: run
run: $(TARGET_FILE)
	PYTHONPATH=$(makefile_dir) ./tools/run ./$(TARGET_FILE)
