import argparse
import hashlib
import subprocess
import sys
from resea.run import run_emulator
from resea.helpers import info, notice, error, plan, progress
from resea.commands.build import build, add_build_arguments
from resea.var import get_var


SHORT_HELP = "build and run"
LONG_HELP = """
Usage: resea run
"""


def run(args):
    build(args)

    plan('Generating a disk image')
    cmd = [get_var('HAL_GENIMAGE'), get_var('EXECUTABLE_PATH'),
           get_var('BUILD_DIR') + '/disk.img']
    progress(' '.join(cmd))

    subprocess.Popen(' '.join(cmd), shell=True).wait()

    plan('Launching an emulator')
    cmd = [get_var('HAL_RUN'), get_var('BUILD_DIR') + '/disk.img']
    progress(' '.join(cmd))
    run_emulator(cmd)

def main(args):
    parser = argparse.ArgumentParser(prog='resea run',
                                     description='run an executable')
    parser = add_build_arguments(parser)
    run(parser.parse_args(args))
