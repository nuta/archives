from reseasdk.helpers import import_module
from reseasdk import command_list


def main(args):
    try:
        cmd = args[0]
    except IndexError:
        cmd = ''

    if cmd == '':
        print("Usage: reseasdk command")
        print("")
        print("Commands:")

        for c in command_list:
            m = import_module("reseasdk.commands.{}".format(c))
            print("  {:<10}{}".format(c, m.SHORT_HELP))
    else:
        m = import_module("reseasdk.commands.{}".format(cmd))
        print(m.LONG_HELP.strip() + "\n")
