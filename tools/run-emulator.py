#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
import re
import colorama

def prettify_line(line):
    m = re.search(r"> (?P<level>INFO|DEBUG|WARN|PANIC|BUG): (?P<body>.*)", line)
    if m:
        # A printk message.
        level = m.group("level")
        body = m.group("body")
        color = {
            "DEBUG": colorama.Fore.YELLOW,
            "INFO": colorama.Fore.BLUE,
            "WARN": colorama.Fore.MAGENTA,
            "BUG": colorama.Fore.RED,
            "PANIC": colorama.Fore.RED,
        }.get(level)

        if level in ["PANIC", "BUG", "WARN"]:
            label = colorama.Style.BRIGHT + level + ": " + colorama.Style.NORMAL
        else:
            label = ""

        return color + label + body + colorama.Style.RESET_ALL
    else:
        return line

def run(argv):
    p = subprocess.Popen(argv, stdin=sys.stdin, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    line = ""
    printk_line = False
    while True:
        data = p.stdout.read(1)
        if len(data) == 0 and p.poll() is not None:
            sys.exit(p.poll())

        ch = data.decode("utf-8", errors="ignore")
        if ch == "\n":
            print(prettify_line(line))
            sys.stdout.flush()
            line = ""
            printk_line = False
        elif printk_line:
            line += ch
        elif len(line) == 0 and ch == ">":
            printk_line = True
            line += ch
        else:
            print(ch, end="")
            sys.stdout.flush()


def main():
    colorama.init()
    run(sys.argv[1:])

if __name__ == "__main__":
    main()
