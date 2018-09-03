# Set 1 for release build.
ENA_RELEASE ?= 1

# The directory to the ena.
ENA_DIR ?=

# avaiable ports: x64
ENA_PORT ?= x64

ENA_CFLAGS += -DENA_PORT_$(ENA_PORT)
ifeq ($(ENA_RELEASE),)
override ENA_CFLAGS += -DENA_DEBUG_BUILD
endif

ENA_SOURCES := \
	$(ENA_DIR)src/api.c \
	$(ENA_DIR)src/lexer.c \
	$(ENA_DIR)src/parser.c \
	$(ENA_DIR)src/eval.c \
	$(ENA_DIR)src/gc.c \
	$(ENA_DIR)src/builtins.c \
	$(ENA_DIR)src/string.c \
	$(ENA_DIR)src/list.c \
	$(ENA_DIR)src/map.c \
	$(ENA_DIR)src/hash.c \
	$(ENA_DIR)src/internal.c \
	$(ENA_DIR)src/malloc.c \
	$(ENA_DIR)src/utils.c \
	$(ENA_DIR)src/port/$(ENA_PORT).c
