from resea.helpers import import_module
from resea import command_list


SHORT_HELP = "print this message"
LONG_HELP = """
Usage: resea help
"""

def main(args):
    try:
        cmd = args[0]
    except IndexError:
        cmd = ''

    if cmd == '':
        print("Usage: resea command")
        print("")
        print("Commands:")

        for c in command_list:
            m = import_module("resea.commands.{}".format(c))
            print("  {:<10}{}".format(c, m.SHORT_HELP))
    else:
        m = import_module("resea.commands.{}".format(cmd))
        print(m.LONG_HELP.strip() + "\n")
