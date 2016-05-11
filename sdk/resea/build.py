import json
import multiprocessing
import os
import shutil
import subprocess
import sys
from termcolor import colored
from resea.package import get_package, load_package_yml, load_packages
from resea.helpers import render, info, notice, error, generating, \
    load_yaml, loads_yaml, dict_to_strdict, plan, progress
from resea.validators import validate_package_yml

MAKEFILE_TEMPLATE = """\
.PHONY: _default
_default: default
# keep blank not to delete intermediate file (especially stub files)
.SECONDARY:
$(VERBOSE).SILENT:

#
#  global config
#
{% for k,v in config.items() %}
{% if k not in ['LANG', 'OBJS', 'DEPS', 'STUBS'] %}
export {{ k }} = {{ v }}
{% endif %}
{% endfor %}

#
#  link
#
default: $(BUILD_DIR)/{{ config['CATEGORY'] }}
$(BUILD_DIR)/{{ config['CATEGORY'] }}: \\
    {% for obj in config['OBJS'] + config['LIBS'] %}
    {{ obj }} \\
    {% endfor %}

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
\t$(CMDECHO) GEN $@
\tWITH_THREADING=yes {{ config['HAL_GENSTART'] }} {{ config['GENSTART_ARGS'] }} > $@
# FIXME: WITH_THREADING is hard coded

#
#  lang
#
{% for lang in config['LANG'].values() %}
#  *.{{ lang['ext'] }}

{% if lang['stub'] %}
STUBS_{{ lang['ext'] }} = \\
{% for stub in config['STUBS'] %}
  $(BUILD_DIR)/stubs/{{ lang['ext'] }}/{{ lang['stub']['prefix'] }}{{ stub }}{{ lang['stub']['suffix'] }} \\
{% endfor %}

$(BUILD_DIR)/stubs/{{ lang['ext'] }}/{{ lang['stub']['prefix'] }}%\
{{ lang['stub']['suffix'] }}: \
packages/%/package.yml
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) GENSTUB $@
\tPACKAGE_NAME=$(PACKAGE_NAME) {{ lang['genstub'] }} $@ $<
{% endif %}

$(BUILD_DIR)/%.o: packages/%.{{ lang['ext'] }} $(BUILD_DIR)/%.deps $(STUBS_{{ lang['ext'] }}) $(BUILD_DIR)/Makefile
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) '{{ lang['abbrev'] }}' $@
\tPACKAGE_NAME=$(PACKAGE_NAME) sh -c '{{ lang['compile'] }} $@ $<'

$(BUILD_DIR)/%.deps: packages/%.{{ lang['ext'] }} $(STUBS_{{ lang['ext'] }}) $(BUILD_DIR)/Makefile
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) 'MKDEPS' $@
\techo "$(@:.deps=.o): `PACKAGE_NAME=$(PACKAGE_NAME) sh -c '{{ lang['mkdeps'] }} $<'`" > $@
{% endfor %}

#
#  local config
#
{% for package_name,config in local_config.items() %}
# {{ package_name }}
$(BUILD_DIR)/{{ package_name }}/%: PACKAGE_NAME = {{ package_name }}
{% for k,v in config.items() %}
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
        p = subprocess.Popen(make.split(' ') + ['-f', makefile],
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
        'MAKEFLAGS': '-j' + str(multiprocessing.cpu_count()),
        'LD_R': '$(LD) -r -o',
        'MKDIR': 'mkdir',
        'CMDECHO': 'echo "-->"',
        'BUILTIN_APPS': [],
        'RESEAPATH': '',
        'PACKAGE': yml['name'],
        'CATEGORY': yml['category']
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

    builtin_packages = [config['PACKAGE'], config['HAL']] + config['BUILTIN_APPS']

    # add kernel to run tests
    if args.env == 'test' and 'kernel' not in builtin_packages:
        builtin_packages.append('kernel')

    if config['CATEGORY'] == 'application' and config['PACKAGE'] not in config['BUILTIN_APPS']:
        config['BUILTIN_APPS'] = config['BUILTIN_APPS'] + [config['PACKAGE']]

    # resolve dependencies
    config, local_config = load_packages(builtin_packages, config, enable_if=True)
    plan('as {CATEGORY} with {HAL} HAL in {BUILD_DIR}'.format(**config))

    # start.o
    config['OBJS'].append(os.path.join(config['BUILD_DIR'], 'start.o'))
    config['START_SOURCE_EXT'] = config['LANG']['c']['ext'] # FIXME
    
    apps = config['BUILTIN_APPS']
    if args.env != 'test' and 'kernel' in apps:
        apps.remove('kernel')

    config['GENSTART_ARGS'] = ' '.join(apps)

    # deps
    config['DEPS'] = list(map(lambda x: os.path.splitext(x)[0] + '.deps', config['OBJS']))
   
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
        for package, c in local_config.items():
            local_config[package] = dict_to_strdict(c)

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

