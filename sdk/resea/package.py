from copy import copy
from collections import OrderedDict
import glob
import os
import sqlite3
import subprocess
import urllib.request
from resea.var import local_config, global_config, Config
from resea.helpers import load_yaml, error, progress
from resea.validators import validate_package_yml

REGISTRY_DB_URL = 'http://resea.net/registry.db'


paths = None # cache
def load_reseapath(base):
    """Looks up for .reseapath in parent directories."""
    global paths

    if paths:
        return paths

    paths = []
    wd = os.getcwd()
    os.chdir(base)
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


def get_package_from_registry(package):
    for d in ['tmp', 'vendor']:
        try:
            os.makedirs(d)
        except FileExistsError:
            pass

    db_path = os.path.join('tmp', 'registry.db')

    # download the registry database
    if not os.path.exists(db_path):
        urllib.request.urlretrieve(REGISTRY_DB_URL, db_path)

    # look for the package
    db = sqlite3.connect(db_path)
    c = db.execute('SELECT type, uri FROM packages WHERE name = ?',(package,))

    r = c.fetchone()
    if r is None:
        error("unknown package '{}' in registry".format(package))

    type_, uri = r

    # download it
    if type_ == 'github':
        name = 'github-{}'.format(uri.replace('/', '-'))
        repo_path = os.path.join('vendor', name)
        if not os.path.exists(repo_path):
            repo_url = 'git://github.com/{}'.format(uri)
            progress('cloning ' + repo_url)
            subprocess.run(['git', 'clone', repo_url, repo_path]) # TODO: use pure python


def get_package_dir(package, search_registry=True):
    """Returns a path to the package."""

    paths = [os.path.abspath('..')]
    for base in ['.'] + glob.glob('vendor/*'):
        paths += load_reseapath(base)

    for path in paths:
        d = os.path.join(path, package)
        if os.path.exists(os.path.join(d, 'package.yml')):
            return d

    if search_registry:
        get_package_from_registry(package)
        return get_package_dir(package, search_registry=False)

    error("package not found: '{}'".format(package))


def _load_include(package, include, config, enable_if):
        include_yml = load_yaml(os.path.join(get_package_dir(package), include))
        try:
            # FIXME
            include_if = not enable_if or ('include_if' in include_yml and \
                eval(include_yml['include_if'], copy(config.getdict())))
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

    ymls = OrderedDict()
    loaded_packages = []
    packages = sorted(builtin_packages.copy())

    # load dependent packages
    while len(packages) > 0:
        package = packages.pop()
        loaded_packages.append(package)

        package_yml_path = os.path.join(get_package_dir(package), 'package.yml')
        yml = load_yaml(package_yml_path, validator=validate_package_yml)

        # include
        for include in yml.get('includes', []):
            for k, v in _load_include(package, include, global_config, enable_if).items():
                # XXX: it looks ugly
                if k in ['config', 'global_config'] and isinstance(v, dict):
                    v = [v]

                if k in yml:
                    if isinstance(v, dict):
                        yml[k].update(v)
                    else:
                        yml[k] += v
                else:
                    yml[k] = v

        yml = validate_package_yml(yml) # yml is modified by include; re-validate it
        ymls[package] = yml

        # update config
        load_global_config(yml.get('global_config', []), enable_if)
        load_local_config(package, yml.get('config', []), enable_if)
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
