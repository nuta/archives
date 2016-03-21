import argparse
from reseasdk.build import build, add_build_arguments

SHORT_HELP = "build an executable"
LONG_HELP = """
Usage: reseasdk build
"""

def main(args):
    parser = argparse.ArgumentParser(prog='reseasdk build',
                                     description='build an executable')
    parser = add_build_arguments(parser)
    build(parser.parse_args(args))
