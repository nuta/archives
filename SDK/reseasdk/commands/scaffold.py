import os
import sys
import argparse
import subprocess
from reseasdk.package import load_package_yml, load_packages
from reseasdk.validators import validate_package_yml
from reseasdk.helpers import load_yaml, error


SHORT_HELP = "generate boilterplate files"
LONG_HELP = """
Usage: reseasdk scaffold
"""


def scaffold(args):
    if not os.path.exists('package.yml'):
        error("'package.yml' not found (are you in a package directory?)")
    yml = load_yaml('package.yml', validator=validate_package_yml)

    config, _ = load_packages(yml['requires'])

    lang = yml.get("lang")
    if lang is None:
        error("lang is not speicified in package.yml")

    bin = config['LANG'][lang].get('scaffold')

    if bin is None:
        error("'{}' lang does not support scaffolding".format(lang))

    subprocess.Popen([bin, ".", "package.yml"]).wait()


def main(args_):
    parser = argparse.ArgumentParser(prog='reseasdk scaffold',
                                     description='Generate boilerplate files')
    args = parser.parse_args(args_)

    scaffold(parser.parse_args(args_))
