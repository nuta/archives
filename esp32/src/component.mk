COMPONENT_OBJS := main.o telemata.o wifi.o uart.o logger.o $(APP_OBJS)
CXXFLAGS += -fdiagnostics-color=always
CPPFLAGS += -DAPP_VERSION=$(APP_VERSION) -DFIRMWARE_VERSION="\"$(FIRMWARE_VERSION)\""

ifeq ($(RELEASE),)
CPPFLAGS += -DDEBUG_BUILD
endif
