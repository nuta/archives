import re


class UndefinedVarError(Exception):
    def __init__(self, k, package=None):
        self.key = k
        self.package = package
    def __str(self):
        return 'undefined variable: {} (package={})'.format(k, repr(package))


class Config:
    def __init__(self):
        self.d = {}

    def getdict(self):
        return self.d

    def _set(self, mode, k, v):
        if mode == 'set':
            self.d[k] = v
        elif mode == 'default':
            self.d.setdefault(k, v)
        elif mode == 'append':
            if isinstance(v, dict):
                self.d[k].update(v)
            else:
                self.d[k] += v
        elif mode == 'append_words':
            self.d[k] = self.d.get(k, '').strip() + ' ' + v.rstrip()

    def set(self, k, v):
        self._set('set', k, v)

    def setdefault(self, k, v):
        self._set('default', k, v)

    def default(self, k, v):
        self._set('default', k, v)

    def append(self, k, v):
        self._set('append', k, v)

    def append_words(self, k, v):
        self._set('append_words', k, v)

    def get(self, k):
        try:
            return self.d[k]
        except KeyError:
            raise UndefinedVarError(k)


global_config = Config()
local_config = {}


def expand_var(val, package=None):
    if isinstance(val, str):
        while True:
            m = re.search('{{[ ]*([a-zA-Z0-9_]+?)[ ]*}}', val)
            if m is None:
                break

            val = val.replace(m.group(0), get_var(m.group(1), package))

    return val


def get_var(k, package=None, default=None):
    configs = None
    if package:
        try:
            configs = [local_config[package], global_config]
        except KeyError:
            pass

    if configs is None:
        configs = [global_config]

    val = None
    for config in configs:
        try:
            val = config.get(k)
            break
        except UndefinedVarError:
            pass

    if val is None:
        if default is None:
            raise UndefinedVarError(k, package)
        else:
            return default

    return expand_var(val, package)
