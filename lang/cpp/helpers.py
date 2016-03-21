import re
import sys

def ctype(type_):
    types = {
        'Data':      'void *',
        'Channel':   'channel_t',
        'Payload':   'payload_t',
        'Id':        'ident_t',
        'Addr':      'uintptr_t',
        'PAddr':     'paddr_t',
        'Offset':    'offset_t',
        'Service':   'service_t',
        'Interface': 'interface_t',
        'Result':    'result_t',
        'UChar':     'uchar_t',
        'Size':      'size_t',
        'Int8':      'int8_t',
        'Int16':     'int16_t',
        'Int32':     'int32_t',
        'Int64':     'int64_t',
        'IntMax':    'intmax_t',
        'UInt8':     'uint8_t',
        'UInt16':    'uint16_t',
        'UInt32':    'uint32_t',
        'UInt64':    'uint64_t',
        'UIntMax':   'uintmax_t',
        '': ''
    }

    if type_ in types.keys():
        return types[type_]

    # Array<Foo>
    m = re.match('Array<([a-zA-Z0-9_]+)>', type_)
    if m is not None:
            return '{}*'.format(ctype(m.group(1)))

    # foo(Bar)
    m = re.match('([a-zA-Z0-9_]+)\(([a-zA-Z0-9_]+)\)', type_)
    if m is not None:
        return '{}_{}'.format(m.group(1), m.group(2))

    sys.exit("error: unknown type: '{}'".format(type_))
