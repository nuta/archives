import os
import sys
import argparse
import subprocess
from resea.package import load_packages
from resea.validators import validate_package_yml
from resea.helpers import load_yaml, error


SHORT_HELP = "generate boilterplate files"
LONG_HELP = """
Usage: resea scaffold
"""


def scaffold(args):
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found (are you in a package directory?)")

    config, _, _ = load_packages([yml['name']] + yml['depends'], {}, update_env=True)

    lang = yml.get("lang")
    if lang is None:
        error("lang is not speicified in package.yml")

    bin = config['LANGS'][lang].get('scaffold')

    if bin is None:
        error("'{}' lang does not support scaffolding".format(lang))

    subprocess.Popen(' '.join([bin, ".", "package.yml"]), shell=True).wait()


def main(args_):
    parser = argparse.ArgumentParser(prog='resea scaffold',
                                     description='Generate boilerplate files')
    args = parser.parse_args(args_)

    scaffold(parser.parse_args(args_))
