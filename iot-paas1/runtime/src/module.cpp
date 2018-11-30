#include "nan.h"

NAN_METHOD(Ioctl);
NAN_METHOD(SerialConfigure);
NAN_METHOD(SpiConfigure);
NAN_METHOD(SpiTransfer);

NAN_MODULE_INIT(init) {
    Nan::SetMethod(target, "ioctl", Ioctl);
    Nan::SetMethod(target, "serialConfigure", SerialConfigure);
    Nan::SetMethod(target, "spiConfigure", SpiConfigure);
    Nan::SetMethod(target, "spiTransfer", SpiTransfer);
}

NODE_MODULE(native, init);
