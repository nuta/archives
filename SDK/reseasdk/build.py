import multiprocessing
import os
import subprocess
import sys
from termcolor import colored
from reseasdk.package import get_package, load_package_yml, load_packages
from reseasdk.helpers import render, info, notice, error, generating, \
    load_yaml, dict_to_strdict, plan, progress
from reseasdk.validators import validate_package_yml

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
{% if k not in ['LANG', 'OBJS', 'STUBS'] %}
export {{ k }} = {{ v }}
{% endif %}
{% endfor %}

#
#  link
#
default: $(BUILD_DIR)/{{ config['CATEGORY'] }}
$(BUILD_DIR)/{{ config['CATEGORY'] }}: \\
    {% for obj in config['OBJS'] %}
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

$(BUILD_DIR)/%.o: packages/%.{{ lang['ext'] }} $(STUBS_{{ lang['ext'] }}) $(BUILD_DIR)/Makefile
\t$(MKDIR) -p $(@D)
\t$(CMDECHO) '{{ lang['abbrev'] }}' $@
\tPACKAGE_NAME=$(PACKAGE_NAME) sh -c '{{ lang['compile'] }} $@ $<'
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
"""


def run_make(makefile, env, prettify=False):
    """Executes make(1)"""
    try:
        p = subprocess.Popen(['make', '-f', makefile],
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
                print('{:<8} {}'.format(
                      colored(cmd, 'magenta', attrs=['bold']),
                      colored(rest, 'yellow')))
            except ValueError:
                print(l)
        else:
            print(l)

    return p.wait()


def get_cmdline_config(args):
    cmdline_config = {}
    for arg in args:
        if '=' not in arg:
            error('invalid default variable (should be FOO=bar form): {}'.format(c))
        k, v = arg.split('=', 1)
        cmdline_config[k] = v
    return cmdline_config


def build(args):
    """Builds an executable."""

    # load package.yml in the current directory
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found (are you in a package directory?)")

    config = {
        'ENV': args.env,
        'BUILD_DIR': 'build/' + args.env,
        'MAKEFLAGS': '-j' + str(multiprocessing.cpu_count()),
        'LD_R': '$(LD) -r -o',
        'MKDIR': 'mkdir',
        'CMDECHO': 'echo "-->"',
        'BUILTIN_APPS': '',
        'RESEAPATH': '',
        'PACKAGE': yml['name']
    }
    cmdline_config = get_cmdline_config(args.config)
    config.update(cmdline_config)

    plan('Building {PACKAGE} ({ENV})'.format(**config))

    if 'HAL' not in config:
        error('HAL is not speicified')

    # TODO: use json in BUILTIN_APPS
    builtin_packages = [config['PACKAGE'], config['HAL']] + \
        list(filter(lambda x: x != '', config['BUILTIN_APPS'].split(',')))

    # add kernel to run tests
    if args.env == 'test' and 'kernel' not in builtin_packages:
        builtin_packages.append('kernel')

    if config['PACKAGE'] not in config['BUILTIN_APPS']:
        config['BUILTIN_APPS'] = ",".join(config['BUILTIN_APPS'].split(',') + [config['PACKAGE']])

    # resolve dependencies
    config, local_config = load_packages(builtin_packages, config)
    plan('as {CATEGORY} with {HAL} HAL in {BUILD_DIR}'.format(**config))

    # start.o
    config['OBJS'].append(os.path.join(config['BUILD_DIR'], 'start.o'))
    config['START_SOURCE_EXT'] = config['LANG']['c']['ext'] # FIXME
    
    apps = config['BUILTIN_APPS'].split(',')
    if args.env != 'test' and 'kernel' in apps:
        apps.remove('kernel')

    config['GENSTART_ARGS'] = ' '.join(apps)

    # override global config with command line variables
    config.update(cmdline_config)
    
    # generate the build directory
    if not os.path.exists(config['BUILD_DIR']):
        os.makedirs(config['BUILD_DIR'], exist_ok=True)

    # generate makefile if needed
    makefile = config['BUILD_DIR'] + '/Makefile'
    if args.r or not os.path.exists(makefile):
        with open(config['BUILD_DIR'] + '/Makefile', 'w') as f:
            f.write(render(MAKEFILE_TEMPLATE, locals()))

    # Everything is ready now. Let's start building!
    progress('executing make')
    if run_make(makefile, os.environ.copy().update(dict_to_strdict(config)),
            args.prettify) != 0:
        error('something went wrong in make(1)')

    return config


def add_build_arguments(parser):
    """Add argparse parser arguments used in building."""
    parser.add_argument('-r', action='store_true', help='regenerate Makefile')
    parser.add_argument('--env', default='release', help='environment')
    parser.add_argument('--prettify', action='store_false', help="prettify output")
    parser.add_argument('--all-in-one', action='store_true', help='embed all required applications')
    parser.add_argument('--single-app', action='store_true', help='no threading')
    parser.add_argument('config', nargs='*', help='config variables (FOO=bar)')
    return parser

