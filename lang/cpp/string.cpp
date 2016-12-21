#include <strfmt.h>
#include <string.h>
#include <string>


string::string() {

    init();
}


string::string(const char *s) {

    init();

    if (s)
        append(s);
}


string::~string() {

    if (_buffer) {
        delete[] _buffer;
        _buffer = nullptr;
    }
}


const char string::at(size_t pos) const {

    if (pos >= _length)
        return 0;

    return _buffer[pos];
}


void string::reserve(size_t n) {
    char *old_buffer = _buffer;
    _capacity = n;
    _buffer = new char[_capacity];
    memcpy_s(_buffer, _capacity, old_buffer, _length);
    delete[] old_buffer;
}


string& string::append(const char *s) {

    return append(s, SIZE_MAX);
}


string& string::append(const char *s, size_t n) {

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


string& string::append(char c) {

    char s[2] = {c, '\0'};
    return append((const char *) &s);
}


size_t string::find(const char *s, size_t pos) const {

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


size_t string::find(char c, size_t pos) const {

    if (!_buffer)
        return npos;

    while (pos <= _length) {
        if(_buffer[pos] == c)
            return pos;

        pos++;
    }

    return npos;
}


bool string::startswith(const char *s) const {

    for (size_t i = 0; *s; i++, s++) {
        if (_buffer[i] != *s)
            return false;
    }

    return true;
}


int string::to_int() const {

    // TODO: support hex
    return atol(c_str());
}


void string::init() {

    _buffer   = nullptr;
    _length   = 0;
    _capacity = 0;
}


void string::fmt(const char *format, ...) {
    va_list vargs;

    va_start(vargs, format);

    vstrfmt(format, vargs, [](void *_s, char c) {
        string *s = (string *) _s;
        s->append(c);
    }, this);

    va_end(vargs);
}
