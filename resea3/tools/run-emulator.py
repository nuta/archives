#!/usr/bin/env python3
import argparse
import os
import subprocess
import threading
import sys
import shlex
import signal
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
            label = colorama.Style.BRIGHT + level + ": "
        else:
            label = ""

        return color + label + body + colorama.Style.RESET_ALL
    else:
        return line

def run(args):
    p = subprocess.Popen(shlex.split(args.command), stdin=sys.stdin, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    def timeout_handler():
        if args.test:
            print("timeout :(")
            p.kill()
    try:
        timeout_thread = threading.Timer(7, timeout_handler)
        timeout_thread.start()

        passed = 0
        failed = 0
        line = ""
        printk_line = False
        while True:
            data = p.stdout.read(1)
            exit_code = p.poll()
            if len(data) == 0 and exit_code is not None:
                timeout_thread.cancel()
                if exit_code == -signal.SIGSEGV:
                    sys.exit( colorama.Style.BRIGHT + colorama.Fore.RED + \
                        "The process caused segmentation fault." + colorama.Style.RESET_ALL)
                sys.exit(1)

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
                        timeout_thread.cancel()
                        p.wait()
                        sys.exit()
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
    except KeyboardInterrupt:
        p.kill()
        timeout_thread.cancel()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--test", action="store_true")
    parser.add_argument("command")
    args = parser.parse_args()

    colorama.init()
    run(args)

if __name__ == "__main__":
    main()
