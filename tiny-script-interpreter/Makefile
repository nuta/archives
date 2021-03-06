VERSION = 0.0.0

include ena.mk
# Set 1 to verbose output.
V =
# Set 1 to build with coverage collector.
COVERAGE =
# Test files.
TESTS =

BENCHMARKS ?= fib startup_time print_loop

ifeq ($(shell uname), Darwin)
LLVM_PREFIX = /usr/local/opt/llvm/bin/
endif

all: ena

PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"
override LD = $(CC)
override LDFLAGS =
override CFLAGS += \
	$(ENA_CFLAGS) \
	-g3 -Isrc/include -DENA_VERSION='"$(VERSION)"' \
	-std=c11 -Wall -Wextra \
	-Werror=implicit-function-declaration \
	-Werror=int-conversion \
	-Werror=incompatible-pointer-types \
	-Werror=shift-count-overflow \
	-Werror=return-type \
	-Werror=shadow

ifneq ($(COVERAGE),)
override CFLAGS += -fprofile-instr-generate -fcoverage-mapping
override LDFLAGS += -fprofile-instr-generate -fcoverage-mapping
endif


SOURCES := $(ENA_SOURCES)
ifeq ($(ENA_RELEASE),)
override CFLAGS += -DENA_WITH_TEST
SOURCES += src/test.c
endif

ifeq ($(ENA_PORT), emscripten)
SOURCES := $(ENA_SOURCES) src/port/emscripten.c
CC := emcc
override CFLAGS += \
	-s "SINGLE_FILE=1" \
	-s SAFE_HEAP=1 \
	-s "EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']" \
	 -s "EXPORTED_FUNCTIONS=['_ena_create_vm', '_ena_create_module', '_ena_register_module', '_ena_eval', '_ena_get_error_cstr']"
endif

$(V).SILENT:
.SECONDARY:
.SUFFIXES:
.PHONY: build test coverage benchmark autotest clean doxygen scan-build valgrind wasm
build: ena

clean:
	rm -rf $(wildcard src/*.o) libena.a ena compile_commands.json doxygen

test: ena
	./ena --test
	./tools/test.py $(TESTS)

coverage:
	$(MAKE) clean
	$(MAKE)	COVERAGE=1 ena
	mkdir -p coverage
	@for test in --test test/*.ena; do \
		LLVM_PROFILE_FILE=coverage/$$(basename -- $$test).profraw ./ena $$test; \
	done
	$(PROGRESS) MERGE coverage/ena.profdata
	$(LLVM_PREFIX)llvm-profdata merge -sparse coverage/*.profraw -o coverage/ena.profdata
	$(PROGRESS) GEN coverage/index.html
	$(LLVM_PREFIX)llvm-cov show -format=html -instr-profile=coverage/ena.profdata ena > coverage/index.html
	$(PROGRESS) REPORT coverage/ena.profdata
	$(LLVM_PREFIX)llvm-cov report -instr-profile=coverage/ena.profdata ena

benchmark:
	$(PROGRESS) MAKE CLEAN
	$(MAKE) clean
	$(PROGRESS) MAKE ENA_RELEASE=1
	$(MAKE) ENA_RELEASE=1
	$(PROGRESS) BENCHMARK
	cd benchmark && ./benchmark.py --verbose $(BENCHMARKS) -o results.json

autotest:
	-make
	-./tools/test.py --ignore-fails $(TESTS)
	$(PROGRESS) "Watching changes to source files..."
	watchmedo shell-command \
		--patterns="*.c;*.h;*.ena;*.py" \
		--ignore-patterns="*.vscode/*" \
		--recursive \
		--command="echo =============================; make -j8 && ./tools/test.py --ignore-fails $(TESTS)"

doxygen:
	$(PROGRESS) doxygen
	cd tools/doxygen && doxygen

scan-build:
	@if [ ! -d clang-static-analyzer ]; then \
		wget https://clang-analyzer.llvm.org/downloads/checker-279.tar.bz2; \
		tar xvf checker-279.tar.bz2; \
		mv checker-279 clang-static-analyzer; \
		rm checker-279.tar.bz2; \
	fi
	make clean
	./clang-static-analyzer/bin/scan-build --view make -j8

valgrind:
	$(PROGRESS) DOCKER_BUILD ena-valgrind
	docker build -t ena-valgrind -f tools/valgrind/Dockerfile .
	$(PROGRESS) DOCKER_RUN ena-valgrind
	docker run -v $(PWD):/ena -it ena-valgrind sh -c "cd /ena && make clean && make -j2 && valgrind ./ena $(TEST)"
	make clean

wasm:
	make ENA_RELEASE=1 ENA_PORT=emscripten ena.js

ena: libena.a src/main.o Makefile
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) -o $@ src/main.o libena.a

ena.js: $(SOURCES:.c=.o) Makefile
	$(PROGRESS) EMCC $@
	$(CC) -o $@ $(SOURCES:.c=.o)

libena.a: $(SOURCES:.c=.o) Makefile
	$(PROGRESS) AR $@
	ar rcs $@ $(SOURCES:.c=.o)

%.o: %.c $(wildcard src/*.h) Makefile
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<
