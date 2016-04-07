import argparse
import hashlib
import os
import subprocess
import sys
from reseasdk.run import run_emulator
from reseasdk.helpers import info, notice, error, plan, progress
from reseasdk.commands.build import build, add_build_arguments


SHORT_HELP = "build and run"
LONG_HELP = """
Usage: reseasdk run
"""


def run(args):
    config = build(args)

    plan('Generating a disk image')
    cmd = [config['HAL_GENIMAGE'], config['BUILD_DIR'] + '/application',
           config['BUILD_DIR'] + '/disk.img']
    progress(' '.join(cmd))
    subprocess.Popen(cmd).wait()

    plan('Launching an emulator')
    cmd = [config['HAL_RUN'], config['BUILD_DIR'] + '/disk.img']
    progress(' '.join(cmd))
    run_emulator(cmd, env=config)

def main(args):
    parser = argparse.ArgumentParser(prog='reseasdk run',
                                     description='run an executable')
    parser = add_build_arguments(parser)
    run(parser.parse_args(args))
