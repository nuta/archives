from reseasdk import __version__

SHORT_HELP = "print the version"
LONG_HELP = """
Usage: reseasdk version
"""


def main(args):
    print('ReseaSDK version {}'.format(__version__))
