#include <termios.h>
#include <fcntl.h>
#include <stdio.h>

#include "nan.h"


using namespace v8;
using namespace node;

static int getBaudRateSpeed(int baudrate) {
    switch (baudrate) {
        case 0: return B0;
        case 9600: return B9600;
        case 115200: return B115200;
    }

    return -1;
}

static int do_configure(int fd, int baudrate, int databits, int parity) {
    struct termios tty = {};

    int speed = getBaudRateSpeed(baudrate);
    if (speed == -1) {
        return 1;
    }

    if (tcgetattr(fd, &tty) != 0) {
        return 1;
    }

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag  = 0;
    tty.c_oflag  = 0;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &tty);
    return 0;
}

NAN_METHOD(Configure) {
    int argc = info.Length();
    if (argc != 4) {
        Nan::ThrowError("Too few arguemnts.");
        return;
    }

    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("`fd' must be integer.");
        return;
    }

    if (!info[1]->IsInt32()) {
        Nan::ThrowTypeError("`baudrate' must be integer.");
        return;
    }

    if (!info[2]->IsInt32()) {
        Nan::ThrowTypeError("`parity' must be integer.");
        return;
    }

    if (!info[3]->IsInt32()) {
        Nan::ThrowTypeError("`databits' must be integer.");
        return;
    }

    int fd       = info[0]->Int32Value();
    int baudrate = info[1]->Int32Value();
    int parity   = info[2]->Int32Value();
    int databits = info[3]->Int32Value();

    if (do_configure(fd, baudrate, parity, databits) != 0) {
        Nan::ThrowError("Something went wrong with serial port device configuration.");
        return;
    }

    info.GetReturnValue().SetUndefined();
}

NAN_MODULE_INIT(init) {
    Nan::SetMethod(target, "configure", Configure);
}

NODE_MODULE(serial, init);
