#include <strfmt.h>
#include <string.h>
#include <String.hpp>


String::String() {

    init(nullptr);
}


String::String(const char *s) {

    init(s);
}


String::String(const String& s) {

    init(s.c_str());
}


String::~String() {

    free_buffer();
}


void String::free_buffer() {

    if (_buffer) {
        delete[] _buffer;
        _buffer = nullptr;
    }
}


void String::truncate_buffer() {

    if (_capacity > 0) {
        _length = 0;
        _buffer[0] = '\0';
    }

}


String& String::assign(const char *s) {

    truncate_buffer();
    append(s);
    return *this;
}


String& String::assign(const char *s, size_t n) {

    truncate_buffer();
    append(s, n);
    return *this;
}


const char String::at(size_t pos) const {

    if (pos >= _length)
        return 0;

    return _buffer[pos];
}


void String::reserve(size_t n) {
    char *old_buffer = _buffer;
    _capacity = n;
    _buffer = new char[_capacity];
    memcpy_s(_buffer, _capacity, old_buffer, _length);
    delete[] old_buffer;
}


String& String::append(const char *s) {

    return append(s, SIZE_MAX);
}


String& String::append(const char *s, size_t n) {

    size_t s_len = strlen(s);
    if (_capacity - _length < s_len || (_capacity == 0 && s_len == 0)) {
        reserve((_length + s_len + 1) * 2);
    }

    while (*s && n > 0) {
        _buffer[_length] = *s;
        _length++;
        s++;
        n--;
    }

    _buffer[_length] = '\0';
    return *this;
}


String& String::append(char c) {

    char s[2] = {c, '\0'};
    return append((const char *) &s);
}


size_t String::find(const char *s, size_t pos) const {

    if (!_buffer)
        return npos;

    size_t s_len = strlen(s);
    while (pos <= _length) {
        if(strncmp(&_buffer[pos], s, s_len) == 0)
            return pos;

        pos++;
    }

    return npos;
}


size_t String::find(char c, size_t pos) const {

    if (!_buffer)
        return npos;

    while (pos <= _length) {
        if(_buffer[pos] == c)
            return pos;

        pos++;
    }

    return npos;
}


bool String::startswith(const char *s) const {

    for (size_t i = 0; *s; i++, s++) {
        if (_buffer[i] != *s)
            return false;
    }

    return true;
}


int String::to_int() const {

    // TODO: support hex
    return atol(c_str());
}


void String::init(const char *s) {

    _buffer   = nullptr;
    _length   = 0;
    _capacity = 0;

    if (s)
        append(s);
}


void String::fmt(const char *format, ...) {
    va_list vargs;

    va_start(vargs, format);

    vstrfmt(format, vargs, [](void *_s, char c) {
        String *s = (String *) _s;
        s->append(c);
    }, this);

    va_end(vargs);
}
