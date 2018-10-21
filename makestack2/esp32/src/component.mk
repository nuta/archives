COMPONENT_OBJS := main.o telemata.o wifi.o uart.o logger.o api.o sakuraio.o
CXXFLAGS += -fdiagnostics-color=always
CPPFLAGS += -DAPP_VERSION=$(APP_VERSION) -DFIRMWARE_VERSION="\"$(FIRMWARE_VERSION)\""
