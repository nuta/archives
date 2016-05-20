import argparse
from resea.build import build, add_build_arguments

SHORT_HELP = "build an executable"
LONG_HELP = """
Usage: resea build
"""

def main(args):
    parser = argparse.ArgumentParser(prog='resea build',
                                     description='build an executable')
    parser = add_build_arguments(parser)
    build(parser.parse_args(args))
