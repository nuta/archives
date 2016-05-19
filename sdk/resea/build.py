import json
import multiprocessing
import os
import shutil
import subprocess
import sys
import pickle
from deepdiff import DeepDiff
from termcolor import colored
from resea.install import install_os_requirements
from resea.package import load_packages, get_package_dir
from resea.helpers import render, info, notice, error, generating, \
    load_yaml, loads_yaml, dict_to_strdict, plan, progress
from resea.validators import validate_package_yml

MAKEFILE_TEMPLATE = """\
.PHONY: _default
_default: default
# keep blank not to delete intermediate file (especially stub files)
.SECONDARY:

# disable implicit rules
.SUFFIXES:

$(VERBOSE).SILENT:

#
#  global config
#
{% for k,v in mk_config.items() %}
export {{ k }} = {{ v }}
{% endfor %}

#
#  link
#
default: $(BUILD_DIR)/{{ config['CATEGORY'] }}
$(BUILD_DIR)/{{ config['CATEGORY'] }}: \\
    {% for obj, _, _, _ in config['OBJS'] %}
    {{ obj }} \\
    {% endfor %}
    {% for lib in config['LIBS'] %}
    {{ lib }} \\
    {% endfor %}
    $(BUILD_DIR)/start.o
{% if config['CATEGORY'] == 'application' %}
\t$(CMDECHO) LINK $@
\t$(HAL_LINK) $@ $^
{% else %}
\t$(CMDECHO) LD_R $@
\t$(LD_R) $@ $^
{% endif %}

#
#  start
#
$(BUILD_DIR)/start.{{ config['START_SOURCE_EXT'] }}:
\t$(CMDECHO) GENSTART $@
\tWITH_THREADING=yes {{ config['HAL_GENSTART'] }} {{ config['GENSTART_ARGS'] }} > $@
# FIXME: WITH_THREADING is hard coded

#
#  lang
#
{% for lang in config['LANGS'].values() %}
#  *.{{ lang['ext'] }}

{% if lang['stub'] %}
STUBS_{{ lang['ext'] }} = \\
{% for stub, _ in config['STUBS'] %}
  $(BUILD_DIR)/stubs/{{ lang['ext'] }}/{{ lang['stub']['prefix'] }}{{ stub }}{{ lang['stub']['suffix'] }} \\
{% endfor %}

{% for stub, package_yml in config['STUBS'] %}
$(BUILD_DIR)/stubs/{{ lang['ext'] }}/{{ lang['stub']['prefix'] }}{{stub }}{{ lang['stub']['suffix'] }}: \
    {{ package_yml }}
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) GENSTUB $@
\tPACKAGE_NAME=$(PACKAGE_NAME) {{ lang['genstub'] }} $@ $<
{% endfor %}

{% endif %}

{% for obj, src, ext, deps in config['OBJS'] %}
{% if ext == lang['ext'] %}
{{ obj }}: {{ src }} {{ deps }} $(STUBS_{{ lang['ext'] }}) $(BUILD_DIR)/Makefile
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) '{{ lang['abbrev'] }}' $@
\tPACKAGE_NAME=$(PACKAGE_NAME) sh -c '{{ lang['compile'] }} $@ $<'

{{ deps }}: {{ src }} $(STUBS_{{ lang['ext'] }}) $(BUILD_DIR)/Makefile
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) 'MKDEPS' $@
\techo "$(@:.deps=.o): `PACKAGE_NAME=$(PACKAGE_NAME) sh -c '{{ lang['mkdeps'] }} $<'`" > $@
{% endif %}
{% endfor %}

{% endfor %}

#
#  local config
#
{% for package_name, mk_config in mk_local_config.items() %}
# {{ package_name }}
$(BUILD_DIR)/{{ package_name }}/%: PACKAGE_NAME = {{ package_name }}
{% for k,v in mk_config.items() %}
$(BUILD_DIR)/{{ package_name }}/%: {{ k }} = {{ v }}
{% endfor %}
{% endfor %}

#
#  deps
#
{% for path in config['DEPS'] %}
-include {{ path }}
{% endfor %}
"""


def run_make(make, makefile, env, prettify=False):
    """Executes make(1)"""
    try:
        p = subprocess.Popen(make.split(' ') + ['-r', '-f', makefile],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT,
                             env=env)
    except Exception as e:
        error('failed to execute make: ' + str(e))

    while True:
        l = p.stdout.readline().decode('utf-8').rstrip()
        if l == '':
            break
        if prettify and l.startswith('--> '):
            try:
                cmd, rest = l.lstrip('--> ').split(' ', 1)
                print(colored('{:<8}'.format(cmd), 'magenta', attrs=['bold']),
                      colored(rest, 'yellow'))
            except ValueError:
                print(l)
        else:
            print(l)

    return p.wait()


def get_cmdline_config(args):
    cmdline_config = {}
    for arg in args:
        if '=' not in arg:
            error('invalid default variable (should be "VAR_NAME=yaml" form): {}'.format(arg))
        k, v = map(lambda x: x.strip(), arg.split('=', 1))
        cmdline_config[k] = loads_yaml(v)
    return cmdline_config


def is_object_equals_to_pickle(obj, pickle_path):
    try:
        p = pickle.load(open(pickle_path, 'rb'))
    except EOFError:
        p = None
    except FileNotFoundError:
        p = None

    return DeepDiff(obj, p) == {}


def build(args, config):
    """Builds an executable."""

    # load package.yml in the current directory
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found (are you in a package directory?)")

    config.setdefault('ENV', 'release')

    default_config = {
        'MAKE': 'make',
        'BUILD_DIR': 'build/' + config['ENV'],
        'EXECUTABLE_PATH': 'build/' + config['ENV'] + '/application',
        'MAKEFLAGS': '-j' + str(multiprocessing.cpu_count()),
        'LD_R': '$(LD) -r -o',
        'MKDIR': 'mkdir',
        'CMDECHO': 'echo "-->"',
        'BUILTIN_APPS': [],
        'RESEAPATH': '',
        'PACKAGE': yml['name'],
    }

    for k, v in default_config.items():
        config.setdefault(k, v)

    configsets_dir = os.path.join(os.path.dirname(__file__), '..', 'configsets')
    for configset in args.configset:
        path = os.path.join(configsets_dir, configset + '.yml')
        config.update(load_yaml(path)['global_config'])

    cmdline_config = get_cmdline_config(args.config)
    config.update(cmdline_config)

    plan('Building {PACKAGE} ({ENV})'.format(**config))

    if 'HAL' not in config:
        error('HAL is not speicified')

    # resolve dependencies
    _config, local_config, ymls = load_packages([yml['name'], config['HAL']] + config.get('BUILTIN_APPS', []),
                                                config, enable_if=True, update_env=True)
    config.update(_config)

    # install os requirements
    os_requirements_pickle = os.path.join(config['BUILD_DIR'], 'os_requirements.pickle')
    os_requirements = list(map(lambda y: y['os_requirements'], ymls.values()))
    if not is_object_equals_to_pickle(os_requirements, os_requirements_pickle):
        for x in os_requirements:
            install_os_requirements(x)

    # add kernel to run tests
    if config['ENV'] == 'test' and 'kernel' not in config['BUILTIN_APPS']:
        config['BUILTIN_APPS'].append('kernel')

    plan('as {CATEGORY} with {HAL} HAL in {BUILD_DIR}'.format(**config))

    config['DEPS'] = []
    config['OBJS'] = []
    config['LIBS'] = []
    for package in sorted(local_config):
        config['LIBS'] += local_config[package].get('LIBS', [])

        for src in local_config[package].get('SOURCES', []):
            base = os.path.splitext(src)[0]
            config['OBJS'].append((
                os.path.join(config['BUILD_DIR'], package, base + '.o'),
                os.path.join(get_package_dir(package), src),
                os.path.splitext(src)[1].replace('.', ''),
                os.path.join(config['BUILD_DIR'], package, base + '.deps')
            ))
            config['DEPS'].append(os.path.join(config['BUILD_DIR'], package, base + '.deps'))

    # start.o
    config['GENSTART_ARGS'] = ' '.join(filter(lambda x: x != 'kernel', config['BUILTIN_APPS']))
    config['START_SOURCE_EXT'] = config['LANGS'][config['HAL_START_LANG']]['ext']
    config['OBJS'].append((
        os.path.join(config['BUILD_DIR'], 'start.o'),
        os.path.join(config['BUILD_DIR'], 'start.' + config['START_SOURCE_EXT']),
        config['START_SOURCE_EXT'],
        os.path.join(config['BUILD_DIR'], 'start.deps'),
    ))

    config['DEPS'].append(os.path.join(config['BUILD_DIR'], 'start.deps'))

    # clean up if build config have been changed
    buildconfig_pickle = os.path.join(config['BUILD_DIR'], 'buildconfig.pickle')
    if os.path.exists(config['BUILD_DIR']) and not is_object_equals_to_pickle(config, buildconfig_pickle):
        plan('detected build config changes; cleaning the build directory')
        progress('deleting {}'.format(config['BUILD_DIR'])) 
        shutil.rmtree(config['BUILD_DIR'])

    # generate the build directory
    if not os.path.exists(config['BUILD_DIR']):
        os.makedirs(config['BUILD_DIR'], exist_ok=True)

    # save the build config and the os requirements to detect changes
    pickle.dump(config, open(buildconfig_pickle, 'wb'))
    pickle.dump(os_requirements, open(os_requirements_pickle, 'wb'))

    # generate makefile if needed
    makefile = config['BUILD_DIR'] + '/Makefile'
    if not os.path.exists(makefile):
        mk_config = dict_to_strdict(config)
        mk_local_config = {}
        for package, c in local_config.items():
            mk_local_config[package] = dict_to_strdict(c)

        with open(config['BUILD_DIR'] + '/Makefile', 'w') as f:
            f.write(render(MAKEFILE_TEMPLATE, locals()))

    # Everything is ready now. Let's start building!
    progress('executing make')
    if run_make(config['MAKE'], makefile,
            os.environ.copy().update(dict_to_strdict(config)),
            args.prettify) != 0:
        error('something went wrong in make(1)')

    return config


def add_build_arguments(parser):
    """Add argparse parser arguments used in building."""
    parser.add_argument('--prettify', action='store_false', help="prettify output")
    parser.add_argument('--configset', nargs="*", default=[], help='config sets')
    parser.add_argument('config', nargs='*', help='config variables (FOO=bar)')
    return parser
