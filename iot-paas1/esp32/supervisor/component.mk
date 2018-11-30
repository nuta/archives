COMPONENT_DIR := $(PWD)/supervisor
TOOLS_DIR := $(PWD)/tools
PLUGINS_DIR := $(PWD)/plugins
PLUGINS := aqm0802a
COMPONENT_OBJS := main.o engine.o builtins.o smms.o supervisor.o wifi.o logger.o

TSC = cd $(PLUGINS_DIR) && yarn --silent run tsc
TSCFLAGS = --target es5 --lib es5 --removeComments

PLUGIN_JS_SRCS = $(addsuffix .js, $(addprefix $(PLUGINS_DIR)/dist/, $(PLUGINS)))
$(PLUGIN_JS_SRCS): %: $(wildcard $(PLUGINS_DIR)/*.ts)
	@echo TSC $@
	$(TSC) $(TSCFLAGS) --outFile $@ $(PLUGINS_DIR)/$(basename $(notdir $@)).ts $(PLUGINS_DIR)/makestack.d.ts

$(PLUGINS_DIR)/dist/builtins.js: $(PLUGINS_DIR)/builtins.ts
	@echo TSC $@
	$(TSC) $(TSCFLAGS) --outFile $@ $^

$(COMPONENT_DIR)/preload.h: $(PLUGINS_DIR)/dist/builtins.js $(PLUGIN_JS_SRCS)
	@echo FILE2C $@
	cat $^ | $(TOOLS_DIR)/file2c preload > $@

engine.o: $(COMPONENT_DIR)/preload.h
