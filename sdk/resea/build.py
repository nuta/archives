import json
import multiprocessing
import os
import shutil
import subprocess
import sys
from termcolor import colored
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
\t$(CMDECHO) GENSTARTC $@
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
            error('invalid default variable (should be "VAR_NAME=yaml" form): {}'.format(c))
        k, v = map(lambda x: x.strip(), arg.split('=', 1))
        cmdline_config[k] = loads_yaml(v)
    return cmdline_config


def build(args):
    """Builds an executable."""

    # load package.yml in the current directory
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found (are you in a package directory?)")

    config = {
        'MAKE': 'make',
        'ENV': args.env,
        'BUILD_DIR': 'build/' + args.env,
        'EXECUTABLE_PATH': 'build/' + args.env + '/application',
        'MAKEFLAGS': '-j' + str(multiprocessing.cpu_count()),
        'LD_R': '$(LD) -r -o',
        'MKDIR': 'mkdir',
        'CMDECHO': 'echo "-->"',
        'BUILTIN_APPS': [],
        'RESEAPATH': '',
        'PACKAGE': yml['name'],
    }

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
    _config, local_config = load_packages([config['PACKAGE'], config['HAL']],
                                config, enable_if=True, update_env=True)
    config.update(_config)

    # TODO: install os requirements

    # add kernel to run tests
    if args.env == 'test' and 'kernel' not in config['BUILTIN_APPS']:
        config['BUILTIN_APPS'].append('kernel')

    plan('as {CATEGORY} with {HAL} HAL in {BUILD_DIR}'.format(**config))

    config['DEPS'] = []
    config['OBJS'] = []
    for package, _local_config in local_config.items():
        for src in _local_config.get('SOURCES', []):
            base = os.path.splitext(src)[0]
            config['OBJS'].append((
                os.path.join(config['BUILD_DIR'], package, base + '.o'),
                os.path.join(get_package_dir(package), src),
                os.path.splitext(src)[1].replace('.', ''),
                os.path.join(config['BUILD_DIR'], package, base + '.deps')
            ))
            config['DEPS'].append(os.path.join(config['BUILD_DIR'], package, base, '.deps'))

    # start.o
    config['GENSTART_ARGS'] = ' '.join(filter(lambda x: x != 'kernel', config['BUILTIN_APPS']))
    config['START_SOURCE_EXT'] = config['LANGS']['c']['ext'] # FIXME
    config['OBJS'].append((
        os.path.join(config['BUILD_DIR'], 'start.o'),
        os.path.join(config['BUILD_DIR'], 'start.' + config['START_SOURCE_EXT']),
        config['START_SOURCE_EXT'],
        os.path.join(config['BUILD_DIR'], 'start.deps'),
    ))

    config['DEPS'].append(os.path.join(config['BUILD_DIR'], 'start.deps'))

    # clean up if build config have been changed
    try:
        prev_config = json.load(open(os.path.join(config['BUILD_DIR'], 'buildconfig.json')))
    except FileNotFoundError:
        prev_config = None

    if prev_config and prev_config != config:
        plan('detected build config changes; cleaning the build directory')
        progress('deleting {}'.format(config['BUILD_DIR'])) 
        shutil.rmtree(config['BUILD_DIR'])

    # generate the build directory
    if not os.path.exists(config['BUILD_DIR']):
        os.makedirs(config['BUILD_DIR'], exist_ok=True)

    # save the build config to detect its changes
    json.dump(config, open(os.path.join(config['BUILD_DIR'], 'buildconfig.json'), 'w'))

    # generate makefile if needed
    makefile = config['BUILD_DIR'] + '/Makefile'
    if args.r or not os.path.exists(makefile):
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
    parser.add_argument('-r', action='store_true', help='regenerate Makefile')
    parser.add_argument('--env', default='release', help='environment')
    parser.add_argument('--prettify', action='store_false', help="prettify output")
    parser.add_argument('--single-app', action='store_true', help='no threading')
    parser.add_argument('--configset', nargs="*", default=[], help='config sets')
    parser.add_argument('config', nargs='*', help='config variables (FOO=bar)')
    return parser

