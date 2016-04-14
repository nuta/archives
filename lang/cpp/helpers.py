import re
import sys

def ctype(type_):
    types = {
        'data':      'void *',
        'channel':   'channel_t',
        'payload':   'payload_t',
        'id':        'ident_t',
        'addr':      'uintptr_t',
        'paddr':     'paddr_t',
        'offset':    'offset_t',
        'service':   'service_t',
        'interface': 'interface_t',
        'result':    'result_t',
        'uchar':     'uchar_t',
        'size':      'size_t',
        'int8':      'int8_t',
        'int16':     'int16_t',
        'int32':     'int32_t',
        'int64':     'int64_t',
        'intmax':    'intmax_t',
        'uint8':     'uint8_t',
        'uint16':    'uint16_t',
        'uint32':    'uint32_t',
        'uint64':    'uint64_t',
        'uintmax':   'uintmax_t',
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
        return '{}_{}_t'.format(m.group(1), m.group(2))

    sys.exit("error: unknown type: '{}'".format(type_))
