#include "aqm0802a.h"

AQM0802A::AQM0802A(int addr) : addr(addr) {
    command(0x39);
    command(0x14);
    command(0x74);
    command(0x56);
    command(0x6c);
    command(0x0c);
}

void AQM0802A::command(int cmd) {
    Wire.beginTransmission(addr);
    Wire.write(0x00);
    Wire.write(cmd);
    Wire.endTransmission(addr);
    delayMicroseconds(30);
}

void AQM0802A::write(char ch) {
    Wire.beginTransmission(addr);
    Wire.write(0x40);
    Wire.write(ch);
    Wire.endTransmission(addr);
    delayMicroseconds(30);
}

void AQM0802A::clear() {
    command(0x01);

}
void AQM0802A::update(const char *str) {
    clear();
    for (int i = 0; str[i] != '\0'; i++) {
        command(0x80 + ((i >= 8) ? (0x40 + i - 8) : i));
        write(str[i]);
    }
}
