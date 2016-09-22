c_stubs = $(addprefix $(BUILD_DIR)/stubs/cpp/, $(addsuffix .c, $(interfaces)))
h_stubs = $(addprefix $(BUILD_DIR)/stubs/cpp/resea/, $(addsuffix .h, $(interfaces)))

objs += \
    lang/cpp/logging.o \
    lang/cpp/string.o  \
    lang/cpp/queue.o   \
    lang/cpp/channel.o \
    $(c_stubs:.c=.o)

CPP_GENSTUB = ./lang/cpp/genstub
CFLAGS += -Wall -std=c11
CFLAGS += -Wno-incompatible-library-redeclaration
CXXFLAGS += -Wall -std=c++11 -fno-exceptions -fno-rtti
CPPFLAGS += -Ilang/cpp -I$(BUILD_DIR)/stubs/cpp -I.

$(BUILD_DIR)/stubs/cpp/%.c: interfaces/%.yaml $(CPP_GENSTUB)
	$(CMDECHO) GENSTUB $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) $(CPP_GENSTUB) $< c > $@

$(BUILD_DIR)/stubs/cpp/resea/%.h: interfaces/%.yaml $(CPP_GENSTUB)
	$(CMDECHO) GENSTUB $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) $(CPP_GENSTUB) $< h > $@

%.o: %.c Makefile $(c_stubs) $(h_stubs)
	$(CMDECHO) CC $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o: %.cpp Makefile $(c_stubs) $(h_stubs)
	$(CMDECHO) CXX $@
	$(MKDIR) -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

%.o: %.S Makefile
	$(CMDECHO) CC $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
