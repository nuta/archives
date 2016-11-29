c_stubs = $(addprefix $(BUILD_DIR)/stubs/cpp/, $(addsuffix .c, $(interfaces)))
h_stubs = $(addprefix $(BUILD_DIR)/stubs/cpp/resea/, $(addsuffix .h, $(interfaces)))

objs += \
    lang/cpp/logging.o \
    lang/cpp/cstring.o \
    lang/cpp/string.o  \
    lang/cpp/new.o     \
    lang/cpp/channel.o

stub_objs += $(c_stubs:.c=.o)

CPP_GENSTUB = ./lang/cpp/genstub
override CFLAGS   += -Wall -std=c11 -Wno-incompatible-library-redeclaration
override CXXFLAGS += -Wall -std=c++11 -fno-exceptions -fno-rtti
override CPPFLAGS += -Ilang/cpp -Ilang/cpp/cpp -I$(BUILD_DIR)/stubs/cpp -I.

$(BUILD_DIR)/stubs/cpp/%.c: interfaces/%.yaml $(CPP_GENSTUB)
	$(CMDECHO) GENSTUB $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) $(CPP_GENSTUB) $< c > $@

$(BUILD_DIR)/stubs/cpp/resea/%.h: interfaces/%.yaml $(CPP_GENSTUB)
	$(CMDECHO) GENSTUB $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) $(CPP_GENSTUB) $< h > $@

$(BUILD_DIR)/%.deps: %.c $(MAKEFILES)
	$(CMDECHO) GENDEPS $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MT $(@:.deps=.o) -MF $@ $<

$(BUILD_DIR)/%.deps: %.cpp $(MAKEFILES)
	$(CMDECHO) GENDEPS $@
	$(MKDIR) -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MT $(@:.deps=.o) -MF $@ $<

$(BUILD_DIR)/%.deps: %.S $(MAKEFILES)
	$(CMDECHO) GENDEPS $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MT $(@:.deps=.o) -MF $@ $<

$(BUILD_DIR)/%.o: %.c $(MAKEFILES) $(c_stubs) $(h_stubs)
	$(CMDECHO) CC $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp $(MAKEFILES) $(c_stubs) $(h_stubs)
	$(CMDECHO) CXX $@
	$(MKDIR) -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S $(MAKEFILES)
	$(CMDECHO) CC $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# for C stubs
%.o: %.c $(MAKEFILES) $(c_stubs) $(h_stubs)
	$(CMDECHO) CC $@
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
