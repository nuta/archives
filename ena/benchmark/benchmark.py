#!/usr/bin/env python3
import argparse
import timeit
import multiprocessing
import subprocess
import platform
import json
import sys
from termcolor import cprint

LOOP_COUNT = 5
INTERPRETERS = [
    ("python3", "py"),
    ("ruby", "rb"),
    ("php", "php"),
    ("lua", "lua"),
    ("luajit", "lua"),
    ("node", "js"),
    ("../ena", "ena"),
]

def get_cpu_brand():
    if platform.system() == "Darwin":
        return check_output(["sysctl", "-n", "machdep.cpu.brand_string"])
    else:
        return list(filter(lambda l: l.startswith("model name"),
            open("/proc/cpuinfo").read().split("\n")))[0].split(" ", 2)[2]

def check_output(argv):
    return subprocess.check_output(argv).decode("utf-8").strip()

def run_test(interpreter, test, ext, repeat):
    stmt = f"import subprocess; subprocess.check_output(['{interpreter}', './programs/{test}/{test}.{ext}'])"
    time_usec = min((timeit.repeat(stmt, repeat=repeat, number=1))) * 1000 * 1000
    return time_usec

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("-o")
    parser.add_argument("tests", nargs="+")
    args = parser.parse_args()

    # Get interpreter versions.
    interpreters = {
        "python3": check_output(["python3", "--version"]).split()[1],
        "node": check_output(["node", "--version"]),
        "lua": check_output(["lua", "-v"]).split()[1],
        "luajit": check_output(["luajit", "-v"]).split()[1],
        "ruby": check_output(["ruby", "-v"]).split()[1],
        "php": check_output(["php", "-v"]).split()[1],
        "ena": check_output(["../ena", "-v"]),
    }

    # Run specified tests.
    results = {}
    for test in args.tests:
        results[test] = {}
        for interpreter, ext in INTERPRETERS:
            if args.verbose:
                print(f"{test} ({interpreter})...", file=sys.stderr)

            # Warm up.
            run_test(interpreter, test, ext, 1)

            # Run a benchmark.
            results[test][interpreter] = run_test(interpreter, test, ext, LOOP_COUNT)

    # Print results in the terminal.
    print()
    for name, result in results.items():
        cprint(name, attrs=["bold", "underline"])
        data = "\n".join([f"{interpreter} {time}" for interpreter, time in result.items()])
        subprocess.run(["termgraph"], input=data, encoding="utf-8")

    # Save as a JSON.
    json.dump({
        "platform": platform.platform(),
        "cpu": get_cpu_brand(),
        "num_cpus": multiprocessing.cpu_count(),
        "interpreters": interpreters,
        "results": results,
    }, open(args.o, "w"), indent=4)

if __name__ == "__main__":
    main()
