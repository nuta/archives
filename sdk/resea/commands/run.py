import argparse
import hashlib
import os
import subprocess
import sys
from resea.run import run_emulator
from resea.helpers import info, notice, error, plan, progress
from resea.commands.build import build, add_build_arguments


SHORT_HELP = "build and run"
LONG_HELP = """
Usage: resea run
"""


def run(args):
    config = build(args)

    env = os.environ.copy()
    env.update(config)

    plan('Generating a disk image')
    cmd = [config['HAL_GENIMAGE'], config['BUILD_DIR'] + '/application',
           config['BUILD_DIR'] + '/disk.img']
    progress(' '.join(cmd))

    subprocess.Popen(cmd, env=env).wait()

    plan('Launching an emulator')
    cmd = [config['HAL_RUN'], config['BUILD_DIR'] + '/disk.img']
    progress(' '.join(cmd))
    run_emulator(cmd, env=env)

def main(args):
    parser = argparse.ArgumentParser(prog='resea run',
                                     description='run an executable')
    parser = add_build_arguments(parser)
    run(parser.parse_args(args))
