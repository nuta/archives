libcxx_objs = \
    vendor/libcxx/src/algorithm.o \
    vendor/libcxx/src/system_error.o \
    vendor/libcxx/src/exception.o \
    vendor/libcxx/src/new.o \
    vendor/libcxx/src/ios.o \
    vendor/libcxx/src/string.o \
    vendor/libcxx/src/hash.o \
    vendor/libcxx/src/memory.o \
    vendor/libcxx/src/locale.o \
    vendor/libcxx/src/mutex.o \
    vendor/libcxx/src/stdexcept.o \
    vendor/libcxx/src/utility.o \
    vendor/libcxx/src/vector.o

objs += $(libcxx_objs)