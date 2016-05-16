from copy import copy
import os
from resea.helpers import load_yaml, error
from resea.install import install_os_requirements
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


def get_package(package):
    """Add a package into packages directory."""

    if not os.path.exists('packages/' + package):
        os.makedirs('packages', exist_ok=True)
        path = get_package_dir(package)
        os.chdir('packages')
        os.symlink(path, package)
        os.chdir('..')


package_ymls = {}  # cache
def load_package_yml(package):
    """Loads a package.yml."""
    if package in package_ymls:
        return package_ymls[package]

    get_package(package)
    yml = load_yaml('packages/{}/package.yml'.format(package),
                    validator=validate_package_yml)
    package_ymls[package] = yml
    return yml


def load_packages(builtin_packages, config=None, enable_if=False):
    """Returns packages config"""

    if config is None:
        config = {
            'BUILD_DIR': '',
        }

    packages = copy(builtin_packages)
    all_packages = []
    local_config = {}
    config['OBJS'] = []
    config['LIBS'] = []
    config['STUBS'] = []
    config['LANGS'] = {}
    while len(packages) > 0:
        package = packages.pop()
        all_packages.append(package)
        yml = load_package_yml(package)
        config['{}_DIR'.format(package.upper())] = 'packages/{}'.format(package)

        install_os_requirements(yml['os_requirements'])

        if yml['category'] == 'application':
            config['CATEGORY'] = 'application'
        if config.get('CATEGORY') != 'application' and yml['category'] == 'library':
            config['CATEGORY'] = 'library'

        for include in yml.get('includes', []):
            d = get_package_dir(package)
            include_yml = load_yaml(os.path.join(d, include))
            try:
                include_if = enable_if and eval(include_yml['include_if'], config)
            except Exception as e:
                error("eval(include_if) in {}: {}".format(
                    package, str(e)))

            if include_if:
                for k,v in include_yml.items():
                    if k in 'include_if':
                        continue
                    if isinstance(yml[k], dict):
                        yml[k].update(v)
                    else:
                        yml[k] = v

        # add dependent packages
        for depend in yml['depends']:
            if depend not in all_packages:
                get_package(depend)
                packages.append(depend)
                builtin_packages.append(depend)

        for depend in yml['uses'] + yml['implements']:
            if depend not in all_packages:
                get_package(depend)
                packages.append(depend)
                config['STUBS'].append(depend)

        if package in builtin_packages:
            # load global config
            for cs in yml.get('global_config', []):
                if enable_if and cs.get('if') and not eval(cs['if'], config):
                    continue

                for k,v in cs.items():
                    if k == 'if':
                        continue

                    if k not in config and v.get('default'):
                        config[k] = v['default']
        else:
            if package in builtin_packages:
                builtin_packages.append(package)

    for package in builtin_packages:
        yml = load_package_yml(package)
        config['STUBS'].append(package)

        # load global config
        for cs in yml.get('global_config', []):
            if cs.get('if') and not eval(cs['if'], config):
                continue

            for k,v in cs.items():
                if k == 'if':
                    continue

                if v.get('append'):
                    val = v['append']
                    if isinstance(val, dict):
                        config[k].update(val)
                    else:
                        error("unexpected global_config type: '{}'".format(type(val)))
                elif v.get('append_words'):
                    config[k] = config.get(k, '').strip() + ' ' + v['append_words'].strip()
                elif v.get('default'):
                    pass
                else:
                    error("unsupported global_config: '{}'".format(repr(v)))

        if yml['category'] in ['application', 'library']:
            # load config
            local_config[package] = {}
            for cs in yml.get('config', []):
                if enable_if and cs.get('if') and not eval(cs['if'], config):
                    continue

                for k,v in cs.items():
                    if k == 'if':
                        continue

                    if v.get('append'):
                        local_config[package][k] = config[k] + v['append']
                    elif v.get('set'):
                        if k == 'SOURCES':
                            for src in v['set']:
                                obj = os.path.splitext(src)[0] + '.o'
                                config['OBJS'].append(os.path.join(config['BUILD_DIR'], package, obj))
                        if k == 'LIBS':
                            config['LIBS'] += v['set']
                        else:
                            local_config[package][k] = v['set']
                    else:
                            error("invalid local config: '{}' (expected set or append)".format(k))

    if '__builtins__' in config:
        del config['__builtins__'] # FIXME

    return config, local_config
