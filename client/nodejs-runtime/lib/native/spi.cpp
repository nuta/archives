#include "nan.h"
using namespace v8;
using namespace node;

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

// This lock should be global because multiple `spi_instance` could
// reference same device file.
uv_mutex_t spidev_lock;

int transfer(int fd, unsigned int speed, void *const tx_buffer, void *rx_buffer, size_t size) {
    struct spi_ioc_transfer transfer;
    transfer.tx_buf   = (uintptr_t) tx_buffer;
    transfer.rx_buf   = (uintptr_t) rx_buffer;
    transfer.len      = size;
    transfer.speed_hz = (__u32) speed;
    transfer.bits_per_word = 8;
    transfer.delay_usecs   = 0;

    uv_mutex_lock(&spidev_lock);

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &transfer)) {
        uv_mutex_unlock(&spidev_lock);
        return 1;
    }

    uv_mutex_unlock(&spidev_lock);
    return 0;
}

int configure(int fd, int mode, int bits, int speed, int order) {
    uv_mutex_lock(&spidev_lock);

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
        uv_mutex_unlock(&spidev_lock);
        return 1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
        uv_mutex_unlock(&spidev_lock);
        return 1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        uv_mutex_unlock(&spidev_lock);
        return 1;
    }

    if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &order) == -1) {
        uv_mutex_unlock(&spidev_lock);
        return 1;
    }

    uv_mutex_unlock(&spidev_lock);
    return 0;
}

#else

int transfer(int fd, int speed, void *const tx_buffer, void *rx_buffer, size_t size) {
    return 0;
}

int configure(int fd, int mode, int bits, int speed, int order) {
    return 0;
}

#endif

//
//  transfer(fd: number, speed: number, tx: Buffer, rx: Buffer): Buffer
//
NAN_METHOD(Transfer) {
    int argc = info.Length();
    if (argc != 4) {
        Nan::ThrowError("Too few arguemnts.");
        return;
    }

    int fd = info[0]->Int32Value();
    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("`fd' must be integer.");
        return;
    }

    int speed = info[1]->Int32Value();
    if (!info[1]->IsInt32()) {
        Nan::ThrowTypeError("`speed' must be integer.");
        return;
    }

    Local<Object> tx_buffer = info[2]->ToObject();
    if (!Buffer::HasInstance(tx_buffer)) {
        Nan::ThrowTypeError("`tx' must be a Buffer.");
        return;
    }

    Local<Object> rx_buffer = info[3]->ToObject();
    if (!Buffer::HasInstance(rx_buffer)) {
        Nan::ThrowTypeError("`rx' must be a Buffer.");
        return;
    }

    if (Buffer::Length(tx_buffer) != Buffer::Length(rx_buffer)) {
        Nan::ThrowTypeError("lengths of `tx` and `rx' are not equal.");
        return;
    }

    int r = transfer(fd, speed, Buffer::Data(tx_buffer), Buffer::Data(rx_buffer),
                     Buffer::Length(rx_buffer));
    info.GetReturnValue().Set(r);
}

//
//  configure(fd: number, mode: number, bits: number, speed: number, order: number): void
//
NAN_METHOD(Configure) {
    int argc = info.Length();
    if (argc != 5) {
        Nan::ThrowError("Too few arguemnts.");
        return;
    }

    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("`fd' must be integer.");
        return;
    }

    if (!info[1]->IsInt32()) {
        Nan::ThrowTypeError("`mode' must be integer.");
        return;
    }

    if (!info[2]->IsInt32()) {
        Nan::ThrowTypeError("`bits' must be integer.");
        return;
    }

    if (!info[3]->IsInt32()) {
        Nan::ThrowTypeError("`speed' must be integer.");
        return;
    }

    if (!info[4]->IsInt32()) {
        Nan::ThrowTypeError("`order' must be integer.");
        return;
    }

    int fd    = info[0]->Int32Value();
    int mode  = info[1]->Int32Value();
    int bits  = info[2]->Int32Value();
    unsigned int speed = info[3]->Int32Value();
    int order = info[4]->Int32Value();

    configure(fd, mode, bits, speed, order);
    info.GetReturnValue().SetUndefined();
}

NAN_MODULE_INIT(init) {
    Nan::SetMethod(target, "configure", Configure);
    Nan::SetMethod(target, "transfer", Transfer);
}

NODE_MODULE(spi, init);
