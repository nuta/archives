import sys
import resea
from resea.helpers import import_module, error

def main(args):
    try:
        command = args[0]
    except IndexError:
        command = 'help'

    if command not in resea.command_list:
        error('command not found: {}'.format(command))

    m = import_module('resea.commands.{}'.format(command))

    exit_code = m.main(args[1:])
    if exit_code is None:
        exit_code = 0

    return exit_code
