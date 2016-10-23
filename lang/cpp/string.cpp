#include <string.h>
#include <string>


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


void string::init() {

    buf      = nullptr;
    length   = 0;
    capacity = 0;
}
