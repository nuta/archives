#ifndef __CPP_STRING_HPP__
#define __CPP_STRING_HPP__

#include <types.h>

class String {
private:
    size_t _capacity;
    size_t _length;
    char *_buffer;
    void init(const char *s);
    void free_buffer();

public:

    // The maximum value of size_t. Note that
    // size_t is *un*signed integer.
    static const size_t npos = -1;

    String();
    ~String();
    String(const char *s);
    String(const String& s);
    const char at(size_t pos) const;
    void reserve(size_t n);
    String& append(const char *s);
    String& append(const char *s, size_t n);
    String& append(char c);
    size_t find(const char *s, size_t pos = 0) const;
    size_t find(char c, size_t pos = 0) const;

    String& operator +=(char c) {
        append(c);
        return *this;
    }

    String& operator +=(const char *s) {
        append(s);
        return *this;
    }

    size_t length() const {
        return _length;
    }

    size_t capacity() const {
        return _capacity;
    }

    const char *c_str() const {
        return _buffer;
    }

    // extensions
    bool startswith(const char *s) const;
    int to_int() const;
    void fmt(const char *format, ...);
};

#endif
