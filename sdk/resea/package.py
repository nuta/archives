from copy import copy
import os
from resea.var import local_config, global_config, Config, get_var
from resea.helpers import load_yaml, error
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


def load_global_config(config, enable_if):
    for cs in config:
        if enable_if and cs.get('if') and not eval(cs['if'], copy(global_config.getdict())):
            continue

        for k,v in cs.items():
            if k == 'if':
                continue

            for mode in ['append', 'append_words', 'default']:
                if v.get(mode):
                    global_config._set(mode, k, v[mode])
                    break
            else:
                error("unsupported global config: '{}'".format(repr(v)))


def load_local_config(package, config, enable_if):
    local_config[package] = Config()
    for cs in config:
        if enable_if and cs.get('if') and not eval(cs['if'], copy(global_config.getdict())):
            continue

        for k,v in cs.items():
            if k == 'if':
                continue

            for mode in ['append', 'append_words', 'set']:
                if v.get(mode):
                    local_config[package]._set(mode, k, v[mode])
                    break
            else:
                error("unsupported local config: '{}'".format(repr(v)))


def load_packages(builtin_packages, enable_if=False, update_env=False):
    """Returns packages config"""

    global_config.set('SOURCES', [])
    global_config.set('STUBS', [])
    global_config.set('LANGS', {})
    global_config.set('BUILTIN_APPS', [])

    ymls = {}
    loaded_packages = []
    packages = builtin_packages.copy()

    # load dependent packages
    while len(packages) > 0:
        package = packages.pop()
        loaded_packages.append(package)

        package_yml_path = os.path.join(get_package_dir(package), 'package.yml')
        yml = load_yaml(package_yml_path, validator=validate_package_yml)
        ymls[package] = yml

        # include
        for include in yml.get('includes', []):
            yml.update(load_include(package, include, enable_if))

        # update config
        load_global_config(yml.get('global_config', {}), enable_if)
        load_local_config(package, yml.get('config', {}), enable_if)
        global_config.set(package.upper() + '_DIR', get_package_dir(package))
        global_config.append('STUBS', (package, package_yml_path))

        if package in builtin_packages and yml['category'] == 'application':
            global_config.append('BUILTIN_APPS', [package])

        # follow dependencies
        for depend in yml['uses'] + yml['implements'] + yml['depends']:
            if depend not in loaded_packages:
                packages.append(depend)

    # determine the build type
    categories = set(map(lambda yml: yml['category'], ymls.values()))
    if all(map(lambda cat: cat != 'application', categories)):
        global_config.set('CATEGORY', 'library')
    else:
        global_config.set('CATEGORY', 'application')

    return ymls
