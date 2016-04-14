import re


def is_ool_type(t):
    # TODO: support foo(Bar)
    return t == 'data' or re.match('Array<([a-zA-Z0-9_]+)>', t) is not None
