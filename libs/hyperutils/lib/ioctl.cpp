#include "nan.h"

#ifdef __linux__

#include <sys/ioctl.h>

#else
int ioctl(int fd, unsigned long request, ...) {
    return 0;
}

#endif

using namespace v8;
using namespace node;

#include <stdio.h>
NAN_METHOD(Ioctl) {
    int argc = info.Length();
    if (argc != 2 && argc != 3) {
        Nan::ThrowError("Too few arguemnts.");
        return;
    }

    int fd = info[0]->Int32Value();
    unsigned long request = info[1]->Int32Value();

    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("`fd' must be integer.");
        return;
    }

    if (!info[1]->IsUint32()) {
        Nan::ThrowTypeError("`request' must be unsigned integer.");
        return;
    }

    void *arg;
    if (argc == 2) {
        arg = NULL;
    } else {
        if (info[2]->IsInt32()) {
            arg = reinterpret_cast<void*>(info[2]->IsInt32());
        } else {
            Local<Object> buffer = info[2]->ToObject();
            if (!Buffer::HasInstance(buffer)) {
                Nan::ThrowTypeError("`arg' must be Number or Buffer.");
                return;
            }
    
            arg = Buffer::Data(buffer);
        }
    }

    int r = ioctl(fd, request, arg);
    info.GetReturnValue().Set(r);
}

NAN_MODULE_INIT(init) {
    Nan::SetMethod(target, "ioctl", Ioctl);
}

NODE_MODULE(ioctl, init);