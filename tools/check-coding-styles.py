#!/usr/bin/env python3
import os
import subprocess

JAVASCRIPT_DIRS = [
    'cli',
    'libs/app-runtime',
    'libs/hyperutils',
    'server/ui',
    'linux/initramfs/app'
]

def main():
    cwd = os.getcwd()
    for d in JAVASCRIPT_DIRS:
        subprocess.run(['eslint', d])

    # TODO: css, ruby, python, rust, shell, and C++


if __name__ == '__main__':
    main()
