ENA_PORT = x64
ENA_DIR = ../../
include ../../ena.mk

CFLAGS += -I$(ENA_DIR)src/include

helloworld: main.o $(ENA_SOURCES:.c=.o)
	$(CC) -o $@ $^

$(ENA_SOURCES:.c=.o): %.o: %.c
	$(CC) $(ENA_CFLAGS) -c -o $@ $<
