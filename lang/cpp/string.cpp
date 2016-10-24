#include <string.h>
#include <string>


string::string() {

    init();
}


string::string(const char *s) {

    init();
    append(s);
}


string& string::append(const char *s) {

    if (capacity - length < strlen(s)) {
        char *old_buf = buf;
        capacity = (length + strlen(s)) * 2;
        buf = new char[capacity];
        memcpy_s(buf, capacity, old_buf, length);
        delete old_buf;
    }

    while (*s) {
        buf[length] = *s;
        length++;
        s++;
    }

    buf[length] = '\0';
    return *this;
}


string& string::append(char c) {

    char s[2] = {c, '\0'};
    return append((const char *) &s);
}


size_t string::find(const char *s, size_t pos = 0) const {

    while (pos <= length) {
        if(strcmp(&buf[pos], s) == 0)
            return pos;

        pos++;
    }

    return npos;
}


size_t string::find(char c, size_t pos = 0) const {

    while (pos <= length) {
        if(buf[pos] == c)
            return pos;

        pos++;
    }

    return npos;
}


const char *string::c_str() const {

    return buf;
}


bool string::startswith(const char *s) const {

    for (size_t i = 0; *s; i++) {
        if (buf[i] != *s)
            return false;
    }

    return true;
}


int string::to_int() const {

    // TODO: support hex

    int x = 0;
    for (size_t i = length, j = 0; i > 0; i--, j++) {
        x += i * (10 * j);
    }

    return x;
}


void string::init() {

    buf      = nullptr;
    length   = 0;
    capacity = 0;
}
