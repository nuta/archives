"""

Useful functions

"""
from copy import copy
import os
import shlex
import subprocess
import sys
import yaml
import jinja2
from termcolor import colored, cprint
from resea.validators import ValidationError

def error(msg):
    """Prints an error message and terminate the program."""
    sys.exit(colored('Resea SDK: {}'.format(msg), 'red'))


def notice(msg):
    """Prints an notice."""
    cprint('Resea SDK: {}'.format(msg), 'yellow')


def info(msg):
    """Prints an informational message."""
    cprint(msg, 'blue')

def _print_with_arrow(msg, color, colored_msg=False):
    cprint('==> ', color, attrs=['bold'], end='')
    if colored_msg:
        cprint(msg, color, attrs=['bold'])
    else:
        cprint(msg, attrs=['bold'])

def progress(msg):
    """Prints progress."""
    _print_with_arrow(msg, 'blue')


def plan(msg):
    """Prints an execution plan."""
    _print_with_arrow(msg, 'green')


def success(msg):
    """Notices that all went well."""
    _print_with_arrow(msg, 'green', True)


def fail(msg):
    """Notices that something went wrong."""
    _print_with_arrow(msg, 'red', True)


def _generating(cmd, target):
    """Returns log message for generating something."""

    return '  {}{}{}'.format(colored(cmd, 'magenta'),
                             (' ' * (16 - len(cmd))),
                             target)


def generating(cmd, target):
    """Prints a 'GEN somthing' message. """
    print(_generating(cmd, target))


def exec_cmd(cmd, ignore_failure=False, cwd=None):
    """Executes a command and returns output.

    cmd can be a list (argv) or str. If cmd is str, it will be
    converted by shltex.split().

    If ignore_failure is True, it returns a empty string
    when an error occurred during executing a command instead
    of raising an exception.

    If cwd is not None, it executes the command in the directory.

    >>> exec_cmd('echo hello!')
    'hello!\\n'
    >>> exec_cmd('./bin/sh -c "pwd"', cwd="/")
    '/\\n'
    >>> exec_cmd('exit 1', ignore_failure=True)
    ''
    >>> exec_cmd('this_command_does_not_exist')
    Traceback (most recent call last):
     ...
    FileNotFoundError: [Errno 2] No such file or directory: \
'this_command_does_not_exist'
    """

    if isinstance(cmd, str):
        cmd = shlex.split(cmd)

    if cwd is not None:
        old_cwd = os.getcwd()
        os.chdir(cwd)

    try:
        output = subprocess.check_output(cmd)
    except Exception as e:
        if ignore_failure:
            return ''
        else:
            raise e
    else:
        return output.decode('utf-8')
    finally:
        if cwd is not None:
            os.chdir(old_cwd)


def import_module(module):
    """Returns a module.

    For example, if you do import_module('foo.bar.baz'), it returns
    the 'baz' module instead of 'foo'.
    """
    m = __import__(module)
    for x in module.split('.'):
        m = getattr(m, x)
    return m


def render(tmpl, vars):
    """Renders a template by jinja2."""
    return jinja2.Environment(
        trim_blocks=True,
        lstrip_blocks=True).from_string(tmpl).render(vars)


def load_yaml(path, **kwargs):
    """Loads a yaml file."""
    return loads_yaml(open(path).read(), path=path, **kwargs)
    return yml


def loads_yaml(s, validator=None, path=None):
    """Loads a yaml string."""
    yml = yaml.safe_load(s)
    if validator is not None:
        try:
            yml = validator(yml)
        except ValidationError as e:
            if path is None:
                error("validation error: {}".format(str(e)))
            else:
                error("validation error in '{}': {}".format(path, str(e)))
    return yml

