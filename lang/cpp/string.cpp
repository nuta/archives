#include <string.h>
#include <string>


string::string() {

    init();
}


string::string(const char *s) {

    init();
    append(s);
}


string::~string() {

    if (_buffer)
        delete[] _buffer;
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
    delete old_buffer;
}


string& string::append(const char *s) {

    size_t s_len = strlen(s);
    if (_capacity - _length < s_len || (_capacity == 0 && s_len == 0)) {
        reserve((_length + s_len + 1) * 2);
    }

    while (*s) {
        _buffer[_length] = *s;
        _length++;
        s++;
    }

    _buffer[_length] = '\0';
    return *this;
}


string& string::append(char c) {

    char s[2] = {c, '\0'};
    return append((const char *) &s);
}


size_t string::find(const char *s, size_t pos) const {

    while (pos <= _length) {
        if(strcmp(&_buffer[pos], s) == 0)
            return pos;

        pos++;
    }

    return npos;
}


size_t string::find(char c, size_t pos) const {

    while (pos <= _length) {
        if(_buffer[pos] == c)
            return pos;

        pos++;
    }

    return npos;
}


bool string::startswith(const char *s) const {

    for (size_t i = 0; *s; i++) {
        if (_buffer[i] != *s)
            return false;
    }

    return true;
}


int string::to_int() const {

    // TODO: support hex

    int x = 0;
    for (size_t i = _length, j = 0; i > 0; i--, j++) {
        x += i * (10 * j);
    }

    return x;
}


void string::init() {

    _buffer   = nullptr;
    _length   = 0;
    _capacity = 0;
}
