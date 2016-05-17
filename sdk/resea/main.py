import sys
from resea.helpers import import_module, error


def main(args):
    try:
        command = args[0]
    except IndexError:
        command = 'help'

    try:
        m = import_module('resea.commands.{}'.format(command))
    except ImportError:
        error('command not found: {}'.format(command))

    exit_code = m.main(args[1:])
    if exit_code is None:
        exit_code = 0

    sys.exit(exit_code)
