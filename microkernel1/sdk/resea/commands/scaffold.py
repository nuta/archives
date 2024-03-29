import argparse
import subprocess
from resea.package import load_packages
from resea.validators import validate_package_yml
from resea.helpers import load_yaml, error
from resea.var import get_var, expand_var

SHORT_HELP = "generate boilterplate files"
LONG_HELP = """
Usage: resea scaffold
"""


def scaffold(args):
    try:
        yml = load_yaml('package.yaml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yaml' not found (are you in a package directory?)")

    load_packages([yml['name']] + yml['depends'], {})

    lang = yml.get('lang')
    if lang is None:
        error("lang is not speicified in package.yaml")

    bin = expand_var(get_var('LANGS')[lang]['scaffold'])

    if bin is None:
        error("'{}' lang does not support scaffolding".format(lang))

    subprocess.Popen(' '.join([bin, ".", "package.yaml"]), shell=True).wait()


def main(args_):
    parser = argparse.ArgumentParser(prog='resea scaffold',
                                     description='Generate boilerplate files')
    scaffold(parser.parse_args(args_))
