#ifndef __AQM0802A_H__
#define __AQM0802A_H__

#include <makestack.h>

class AQM0802A {
private:
    int addr;
    void command(int cmd);
    void write(char ch);

public:
    AQM0802A(int addr = 0x3e);
    void clear();
    void update(const char *str);
};

#endif
