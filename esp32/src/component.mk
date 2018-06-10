COMPONENT_OBJS := main.o telemata.o wifi.o uart.o logger.o $(APP_OBJS)
CXXFLAGS += -fdiagnostics-color=always
CPPFLAGS += -DWIFI_SSID="\"$(WIFI_SSID)\"" -DWIFI_PASSWORD="\"$(WIFI_PASSWORD)\"" -DSERVER_URL="\"$(SERVER_URL)\"" \
            -DCA_CERT="\"$(CA_CERT)\"" -DDEVICE_ID="\"$(DEVICE_ID)\"" \
            -DAPP_VERSION=$(APP_VERSION) \
            -DFIRMWARE_VERSION="\"$(FIRMWARE_VERSION)\""

ifeq ($(RELEASE),)
CPPFLAGS += -DDEBUG_BUILD
endif
