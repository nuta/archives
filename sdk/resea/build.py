import json
import multiprocessing
import os
import shutil
import subprocess
import sys
import pickle
from deepdiff import DeepDiff
from termcolor import colored
from resea.var import global_config, local_config, get_var, expand_var, UndefinedVarError
from resea.install import install_os_requirements
from resea.package import load_packages, get_package_dir
from resea.helpers import render, info, notice, error, generating, \
    load_yaml, loads_yaml, plan, progress
from resea.validators import validate_package_yml

MAKEFILE_TEMPLATE = """\
default: {{ target }}

# keep blank not to delete intermediate file (especially stub files)
.SECONDARY:

# disable implicit rules
.SUFFIXES:

$(VERBOSE).SILENT:

MKDIR = mkdir
CMDECHO = echo "-->"

# link
{{ target }}: {{ target_deps | join(' ') }}
	$(CMDECHO) LINK $@
	{{ hal_link }} $@ $^

# auto-generated files
{% for path, cmd in autogen_files %}
{{ path }}:
	$(MKDIR) -p $(@D)
	$(CMDECHO) GEN $@
	{{ cmd }} $@
{% endfor %}

# start
{{ start_file }}:
	$(CMDECHO) GENSTART $@
	{{ genstart }} > $@

# stubs
{% for stub, package_yml, genstub in stubs %}
{{ stub }}: {{ package_yml }}
	$(MKDIR) -p $(@D)
	$(CMDECHO) GENSTUB $@
	{{ genstub }} $@ $<
{% endfor %}

# compile / mkdep
{% for files, abbrev, compile, mkdeps in sources %}
    {% for src, out, deps in files %}
{{ out }}: {{ src }} {{ stub_files | join(' ') }}
	$(MKDIR) -p $(@D)
	$(CMDECHO) '{{ abbrev }}' $@
	{{ compile }} $@ $<

{{ deps }}: {{ src }} {{ stub_files | join(' ') }}
	$(MKDIR) -p $(@D)
	$(CMDECHO) 'MKDEPS' $@
	echo "$(@:.deps=.o): $(shell {{ mkdeps }} $<)" > $@
    {% endfor %}
{% endfor %}

#
#  deps
#
{% for path in deps_files %}
-include {{ path }}
{% endfor %}
"""


def run_make(make, makefile, prettify=False):
    """Executes make(1)"""
    makeflags = ['-j' + str(multiprocessing.cpu_count())]
    try:
        p = subprocess.Popen(make.split(' ') + ['-r', '-f', makefile] +
                             makeflags,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
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


def load_configsets(configsets):
    configsets_dir = os.path.join(os.path.dirname(__file__), '..', 'configsets')
    for configset in configsets:
        path = os.path.join(configsets_dir, configset + '.yml')
        for k, v in load_yaml(path)['global_config'].items():
            global_config.set(k, v)

def load_cmdline_config(args):
    for arg in args:
        for op, func in [('+=', global_config.append_words), ('=', global_config.set)]:
            if op in arg:
                break
        else:
            error('invalid default variable (should be "VAR_NAME=yaml" form): {}'.format(arg))

        k, v = map(lambda x: x.strip(), arg.split(op, 1))

        val = loads_yaml(v)
        if val is None:
            val = ''

        func(k, val)


def is_object_equals_to_pickle(obj, pickle_path):
    try:
        p = pickle.load(open(pickle_path, 'rb'))
    except EOFError:
        p = None
    except FileNotFoundError:
        p = None

    return DeepDiff(obj, p) == {}


def build(args):
    """Builds an executable."""

    # load package.yml in the current directory
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found (are you in a package directory?)")

    global_config.setdefault('ENV', 'release')
    global_config.setdefault('MAKE', 'make')
    global_config.setdefault('TEST', False)
    global_config.set('BUILD_DIR', 'build/' + get_var('ENV'))
    global_config.set('EXECUTABLE_PATH', expand_var('{{ BUILD_DIR }}/application'))

    load_configsets(args.configset)
    load_cmdline_config(args.config)

    if get_var('HAL', default=None) is None:
        error('HAL is not speicified')

    # resolve dependencies
    progress('Loading packages')
    packages = [yml['name'], get_var('HAL')] + get_var('BUILTIN_APPS', default=[])
    if get_var('TEST') and 'kernel' not in packages:
        packages.append('kernel')
    ymls = load_packages(packages, enable_if=True, update_env=True)

    build_dir = get_var('BUILD_DIR')

    plan('Building {CATEGORY} with {HAL} HAL in {BUILD_DIR} ({ENV})'.format(**global_config.getdict()))
    # install os requirements
    os_requirements_pickle = os.path.join(build_dir, 'os_requirements.pickle')
    os_requirements = list(map(lambda y: y['os_requirements'], ymls.values()))
    if not is_object_equals_to_pickle(os_requirements, os_requirements_pickle):
        plan('Installing OS requirements')
        for x in os_requirements:
            install_os_requirements(x)

    sources = []
    autogen_files = []
    libs = []
    stubs = []
    deps_files = []
    stub_files = []
    for package in ymls.keys():
        package_dir = get_package_dir(package)
        libs += get_var('LIBS', package, default=[])

        ext_lang = {}
        for lang_name, lang in get_var('LANGS').items():
            ext_lang[lang['ext']] = lang

            if 'stub' not in lang:
                continue

            stub_file = os.path.join(build_dir, 'stubs', lang_name,
                            lang['stub']['prefix'] + package + lang['stub']['suffix'])
            stubs.append((
                stub_file,
                os.path.join(package_dir, 'package.yml'),
                expand_var(get_var('LANGS', package)['cpp']['genstub'], package)
            ))
            stub_files.append(stub_file)

        source_files = []
        for src in get_var('SOURCES', package, default=[]):
            base, ext = os.path.splitext(src)
            deps_file = os.path.join(build_dir, package, base + '.deps')
            deps_files.append(deps_file)
            sources.append((
                [(os.path.join(get_package_dir(package), src),
                  os.path.join(build_dir, package, base + '.o'),
                  deps_file)],
                expand_var(ext_lang[ext.lstrip('.')]['abbrev'], package),
                expand_var(ext_lang[ext.lstrip('.')]['compile'], package),
                expand_var(ext_lang[ext.lstrip('.')]['mkdeps'], package)
            ))

        for f in get_var('FILES', package, default=[]):
            path = os.path.join(package_dir, f['path'])
            cmd = expand_var(f['cmd'], package)
            autogen_files.append((path, cmd))

    # start
    if get_var('TEST'):
        genstart = ' '.join([get_var('HAL_GENSTART'),
                            '--with-threading',
                             '--test',
                             '--test-target', yml['name'],
                             ' '.join(get_var('BUILTIN_APPS'))])
    else:    
        genstart = ' '.join([get_var('HAL_GENSTART'), ' '.join(get_var('BUILTIN_APPS'))])

    start_file = os.path.join(build_dir, 'start.' + get_var('LANGS')['cpp']['ext'])
    start_deps_file = os.path.join(build_dir, 'start.deps')
    start_obj_file = os.path.join(build_dir, 'start.o')
    lang = get_var('HAL_START_LANG')
    sources.append((
        [(start_file, start_obj_file, start_deps_file)],
        expand_var(get_var('LANGS', package)[lang]['abbrev'], package),
        expand_var(get_var('LANGS', package)[lang]['compile'], package),
        expand_var(get_var('LANGS', package)[lang]['mkdeps'], package)
    ))

    # target
    if get_var('TEST'):
        global_config.set('CATEGORY', 'application')

    category = get_var('CATEGORY')
    hal_link = get_var('HAL_LINK') # TODO support ld -r
    target = os.path.join(build_dir, category)
    target_deps = libs
    for files, _, _, _ in sources:
        for _, obj, _ in files:
            target_deps.append(obj)

    # clean up if build config have been changed
    buildconfig = (global_config.getdict(), [c.getdict() for c in local_config.values()])
    buildconfig_pickle = os.path.join(build_dir, 'buildconfig.pickle')
    if os.path.exists(build_dir) and not is_object_equals_to_pickle(buildconfig, buildconfig_pickle):
         plan('detected build config changes; cleaning the build directory')
         progress('deleting {}'.format(build_dir)) 
         shutil.rmtree(build_dir)

    # generate the build directory
    if not os.path.exists(build_dir):
        os.makedirs(build_dir, exist_ok=True)

    # save the build config and the os requirements to detect changes
    pickle.dump(buildconfig, open(buildconfig_pickle, 'wb'))
    pickle.dump(os_requirements, open(os_requirements_pickle, 'wb'))

    # generate makefile if needed
    makefile = build_dir + '/Makefile'
    if not os.path.exists(makefile):
        with open(makefile, 'w') as f:
            f.write(render(MAKEFILE_TEMPLATE, locals()))

    # Everything is ready now. Let's start building!
    progress('executing make')
    if run_make(get_var('MAKE'), makefile, args.prettify) != 0:
        error('something went wrong in make(1)')


def add_build_arguments(parser):
    """Add argparse parser arguments used in building."""
    parser.add_argument('--prettify', action='store_false', help="prettify output")
    parser.add_argument('--configset', nargs="*", default=[], help='config sets')
    parser.add_argument('config', nargs='*', help='config variables (FOO=bar)')
    return parser
