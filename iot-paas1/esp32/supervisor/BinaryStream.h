#pragma once
#include <Stream.h>

class BinaryStream : public Stream {
private:
    int buffer_size;

    // Returns 1 on success or 0 on failure.
    int reserve(int new_length) {
        const int count = 64;

        if (buffer_size < new_length) {
            buffer_size = new_length + count;
            buffer = (uint8_t *) realloc(buffer, buffer_size);
            if (!buffer) {
                free(buffer);
                return 0;
            }
        }

        return 1;
    }

public:
    int length;
    int read_index;
    uint8_t *buffer;

    BinaryStream() : length(0), read_index(0) {
        buffer_size = 128;
        buffer = (uint8_t *) malloc(buffer_size);
    }

    ~BinaryStream() {
        free(buffer);
    }

    size_t write(const uint8_t *data, size_t size) {
        if(size && data) {
            if(reserve(length + size)) {
                memcpy((void *) (buffer + length), (const void *) data, size);
                length += size;
                return size;
            }
        }
        return 0;
    }

    size_t write(uint8_t data) {
        if (reserve(length + 1)) {
            buffer[length] = data;
            return data;
        }

        return 0;
    }

    int available() {
        return length > 0;
    }

    int read() {
        if(length > 0) {
            char c = buffer[read_index];
            read_index++;
            return c;

        }

        return -1;
    }

    int peek() {
        return ((length > 0) ? buffer[0] : -1);
    }

    void flush() {
    }
};
