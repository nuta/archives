override CFLAGS += -std=c11 -O2 -g3 --target=x86_64 -ffreestanding -fno-builtin -nostdinc -nostdlib -I$(LAYER_DIR)/lib
override LDFLAGS += --script $(LAYER_DIR)/lib/bin.ld

libs := crt stdio stdlib string syscall
libs_objs := $(foreach lib, $(libs), $(LAYER_DIR)/lib/$(lib).o)

$(TARGET): $(SRC:.c=.o) $(libs_objs)
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) -o $@ $^

$(LAYER_DIR)/%.o: $(LAYER_DIR)/%.c
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $^

$(LAYER_DIR)/%.o: $(LAYER_DIR)/%.S
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $^

include common.mk
