#ifndef __CPP_BUFFER_H__
#define __CPP_BUFFER_H__

/* A extended auto_ptr. */
class Buffer {
private:
    void *ptr;
    size_t len;

public:
    Buffer() : ptr(nullptr), len(0) {}
    Buffer(const Buffer &buffer) = delete;
    void operator=(const Buffer &buffer) = delete;

    ~Buffer() {
        free(ptr);
    }

    void *get() {
        return ptr;
    }

    void *release() {
        void *p = ptr;
        ptr = nullptr;
        return p;
    }

    size_t length() {
        return len;
    }

    void set(void *p, size_t l) {
        ptr = p;
        len = l;
    }
};

#endif
