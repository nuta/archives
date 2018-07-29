#!/usr/bin/env python3
import argparse
import os
import subprocess
import threading
import sys
import shlex
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

def run(args):
    p = subprocess.Popen(shlex.split(args.command), stdin=sys.stdin, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    def timeout_handler():
        print("timeout :(")
        p.kill()

    timeout = 3 if args.test else sys.maxsize
    timeout_thread = threading.Timer(timeout, timeout_handler)
    timeout_thread.start()

    passed = 0
    failed = 0
    line = ""
    printk_line = False
    while True:
        data = p.stdout.read(1)
        if len(data) == 0 and p.poll() is not None:
            timeout_thread.cancel()
            sys.exit(p.poll())

        ch = data.decode("utf-8", errors="ignore")
        if ch == "\n":
            if "PASS: " in line:
                passed += 1
            if "FAIL: " in line:
                failed += 1
            if "Finished all tests" in line:
                p.kill()
                if failed == 0:
                    print(colorama.Style.BRIGHT + colorama.Fore.GREEN + f"Passed all {passed} tests.")
                else:
                    print(colorama.Style.BRIGHT + colorama.Fore.RED + f"Failed {failed} tests.")
                continue
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
    parser = argparse.ArgumentParser()
    parser.add_argument("--test", action="store_true")
    parser.add_argument("command")
    args = parser.parse_args()

    colorama.init()
    run(args)

if __name__ == "__main__":
    main()
