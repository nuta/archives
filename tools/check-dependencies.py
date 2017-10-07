#!/usr/bin/env python3
import glob
import tempfile
import sys
import os
import json
import subprocess
from pprint import pprint

exit_code = 0

def progress(msg):
    print(f"\x1b[1;34m==> {msg}\x1b[m")

def error(msg):
    global exit_code
    exit_code = 1
    print(f"\x1b[1;31m==> {msg}\x1b[m")

def main():
    repo_dir = os.getcwd()

    #
    #  Gemfile.lock
    #
    for gemfile_path in glob.glob("**/Gemfile.lock", recursive=True):
        progress(f"Checking {gemfile_path}")
        os.chdir(os.path.dirname(gemfile_path))

        result = subprocess.run(['hakiri', 'gemfile:scan', '--quiet', '--force'],
            stdout=subprocess.PIPE, encoding='utf-8').stdout

        if "!" in result:
            error(f"found issues in `{gemfile_path}'")
            print(result)

        os.chdir(repo_dir)

    #
    #  package.json
    #
    package_json_files = list(map(lambda f: os.path.join(repo_dir, f),
        filter(lambda f: "node_modules" not in f,
            glob.iglob("**/package.json", recursive=True))))

    with tempfile.TemporaryDirectory() as tmpdir:
        os.chdir(tmpdir)

        for package_json_path in package_json_files:
            progress(f"Checking {package_json_path}")
            package_json = json.load(open(package_json_path))
            dependencies = package_json.get('dependencies', {})
            devDependencies = package_json.get('devDependencies', {})
            merged_dependencies = dependencies
            merged_dependencies.update(devDependencies)

            json.dump({
                'name': 'dummy',
                'dependencies': merged_dependencies
            }, open("package.json", 'w'))

            p = subprocess.run(["nsp", "check", "--output", "json"], encoding='utf-8',
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            
            results = json.loads(p.stdout)
            for result in results:
                scope = '(in devDependencies)' if result['module'] not in dependencies else ''
                error(f"found a vulnerability in `{result['module']}' {scope}")
                pprint(result, indent=4)

if __name__ == '__main__':
    main()
    if exit_code != 0:
        sys.exit("some tests failed :(")