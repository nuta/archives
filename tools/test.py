#!/usr/bin/env python3
import argparse
import subprocess
import os
import sys
import threading
from termcolor import cprint

test_proc = None

def kill_process(name):
    if test_proc.poll() is None:
        cprint(f"{name} takes too long, aborting...", "red", attrs=["bold"])
        test_proc.kill()

failed = 0
total = 0
def run_test(filepath):
    global test_proc, failed, total
    print(f"{filepath}...", end="")
    sys.stdout.flush()

    test_proc = subprocess.Popen(
        ['./ena', filepath],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )

    kill_timer = threading.Timer(1, kill_process, args=(filepath,))
    kill_timer.start()
    exit_code = test_proc.wait()
    if exit_code == 0:
        print("PASS")
        kill_timer.cancel()
    else:
        if exit_code == -11:
            cprint("Segumentation Fault", "red", attrs=["bold"])
        else:
            cprint(f"exited with {exit_code}", "red", attrs=["bold"])
        cprint(test_proc.stdout.read().decode("utf-8"), "yellow")
        failed += 1

    total += 1

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--all", action="store_true")
    parser.add_argument("--ignore-fails", action="store_true")
    parser.add_argument("tests", nargs="*")
    args = parser.parse_args()

    if len(args.tests) > 0:
        tests = args.tests
    else:
        tests = []
        for root, dirs, files in os.walk("test"):
            for file_ in files:
                tests.append(os.path.join(root, file_))

    cprint("=" * 50, "cyan", attrs=["bold"])
    for filepath in tests:
        run_test(filepath)

    if failed == 0:
        cprint(f"Passed {total} tests", "green")
    else:
        cprint(f"Failed {failed}/{total} tests", "red", attrs=["bold"])
        if not args.ignore_fails:
            sys.exit(1)

if __name__ == "__main__":
    main()
