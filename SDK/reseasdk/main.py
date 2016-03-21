from reseasdk.helpers import import_module, error


def main(args):
    try:
        command = args[0]
    except IndexError:
        command = 'help'

    try:
        m = import_module('reseasdk.commands.{}'.format(command))
    except ImportError:
        error('command not found: {}'.format(command))

    m.main(args[1:])
