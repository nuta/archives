#!/usr/bin/env python3
import os
import sys
import subprocess

JAVASCRIPT_DIRS = [
    'cli',
    'libs/app-runtime',
    'libs/hyperutils',
    'server/ui',
    'linux/initramfs/app'
]

exit_code = 0
def main():
    global exit_code

    cwd = os.getcwd()
    for d in JAVASCRIPT_DIRS:
        if subprocess.run(['eslint', d]).returncode != 0:
            exit_code = 1

    # TODO: css, ruby, python, rust, shell, and C++

if __name__ == '__main__':
    main()

    if exit_code != 0:
        sys.exit("some lints failed :(")
