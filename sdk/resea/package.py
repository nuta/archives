from copy import copy
import os
from resea.helpers import load_yaml, error, dict_to_strdict
from resea.validators import validate_package_yml, ValidationError

paths = None # cache
def load_reseapath():
    """Looks up for .reseapath in parent directories."""
    global paths

    if paths:
        return paths

    paths = []
    wd = os.getcwd()
    while True:
        cwd = os.getcwd()

        try:
            with open('.reseapath') as f:
                for path in f.readlines():
                    paths.append(os.path.abspath(path.strip()))
        except FileNotFoundError:
            pass

        os.chdir('..')
        if os.getcwd() == cwd:
            # root directory
            break

    os.chdir(wd)
    return paths


def get_package_dir(package):
    """Returns a path to the package."""

    for path in [os.path.abspath('..')] + load_reseapath():
        d = os.path.join(path, package)
        if os.path.exists(os.path.join(d, 'package.yml')):
            return d

    error("package not found: '{}'".format(package))


def _load_include(package, include, config, enable_if):
        include_yml = load_yaml(os.path.join(get_package_dir(package), include))
        try:
            # FIXME
            include_if = not enable_if or eval(include_yml['include_if'], config)
        except Exception as e:
            error("eval(include_if) in {}: {}".format(
                package, str(e)))

        if include_if:
            include_yml.pop('include_if', None)
            yml = include_yml
        else:
            yml = {}

        return yml


def load_global_config(package, config, global_config, enable_if):
    for cs in config:
        if enable_if and cs.get('if') and not eval(cs['if'], global_config):
            continue

        for k,v in cs.items():
            if k == 'if':
                continue

            if v.get('append'):
                val = v['append']
                if isinstance(val, dict):
                    global_config[k].update(val)
                else:
                    error("unexpected global_config type: '{}'".format(type(val)))
            elif v.get('append_words'):
                global_config[k] = global_config.get(k, '').strip() + ' ' + v['append_words'].strip()
            elif v.get('default'):
                global_config.setdefault(k, v['default'])
            else:
                error("unsupported global_config: '{}'".format(repr(v)))

    return global_config


def load_local_config(package, config, global_config, enable_if):
    local_config = {}
    for cs in config:
        if enable_if and cs.get('if') and not eval(cs['if'], global_config):
            continue

        for k,v in cs.items():
            if k == 'if':
                continue

            if v.get('append'):
                if k not in local_config:
                    local_config[k] = global_config[k] + v['append']
                local_config[k] += v['append']
            elif v.get('set'):
                local_config[k] = v['set']
            else:
                error("invalid local config: '{}' (expected set or append)".format(k))

    return local_config


def load_packages(builtin_packages, config, enable_if=False, update_env=False):
    """Returns packages config"""

    config.update({
        'SOURCES':  [],
        'STUBS':    [],
        'LANGS':    {},
        'BUILTIN_APPS': []
    })

    local_config = {}
    ymls = {}
    packages = builtin_packages.copy()
    loaded_packages = []
    categories = []

    # load dependent packages
    while len(packages) > 0:
        package = packages.pop()

        loaded_packages.append(package)
        package_yml_path = os.path.join(get_package_dir(package), 'package.yml')
        yml = load_yaml(package_yml_path, validator=validate_package_yml)

        for include in yml.get('includes', []):
            yml.update(load_include(package, include, config, enable_if))

        config.update(load_global_config(package, yml.get('global_config', {}),
                                         config, enable_if))
        local_config[package] = load_local_config(package, yml.get('config', {}),
                                                  config, enable_if)
        config[package.upper() + '_DIR'] = get_package_dir(package)
        config['STUBS'].append((package, package_yml_path))

        for depend in yml['uses'] + yml['implements'] + yml['depends']:
            if depend not in loaded_packages:
                packages.append(depend)

        if package in builtin_packages and yml['category'] == 'application':
            config['BUILTIN_APPS'].append(package)

        categories.append(yml['category'])
        ymls[package] = yml

    # determine the build type
    if any(map(lambda cat: cat == 'application', set(categories))):
        config['CATEGORY'] = 'application'
    elif any(map(lambda cat: cat == 'library', set(categories))):
        config['CATEGORY'] = 'library'
    else:
        config['CATEGORY'] = 'unknown'

    if '__builtins__' in config:
        del config['__builtins__'] # FIXME

    if update_env:
        os.environ.update(dict_to_strdict(config))

    return config, local_config, ymls
