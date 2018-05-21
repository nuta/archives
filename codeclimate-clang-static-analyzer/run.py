#!/usr/bin/env python3
import glob
import json
import os
import sys
import shlex
import shutil
import subprocess
import plistlib


def main():
    config = json.load(open("/config.json"))["config"]
    script = config.get("script")

    if script is None:
        sys.exit("Error: `script' is not set")

    # Copy files to /tmp/code because /code is readonly.
    shutil.copytree('/code', '/tmp/code')
    os.chdir('/tmp/code')

    # Compile the project with Clang Static Analyzer.
    sys.stderr.write("running '{}'\n".format(script))
    subprocess.run(["scan-build", "-plist", "-o", "results", "sh", "-c", script], check=True,
      stdout=sys.stderr)

    # Parse results.
    plists = glob.glob("results/*/*.plist")
    if plists == []:
        # No issues! yay!
        return

    plist_path = plists[0]
    result = plistlib.load(open(plist_path, "rb"))
    diagnostics = result["diagnostics"]
    files = result["files"]
    issues = []
    for issue in diagnostics:
        issues.append({
            "type": "issue",
            "check_name": "Clang Static Analyzer",
            "description": issue["description"],
            "categories": ["Bug Risk"],
            "severity": "normal",
            "content": { "body": issue["path"][0]["extended_message"] },
            "location": {
                "path": files[issue["location"]["file"]],
                "lines": {
                    "begin": issue["location"]["line"],
                    "end": issue["location"]["line"]
                }
            }
        })

    for issue in issues:
        print("{}\0".format(json.dumps(issue)))


if __name__ == "__main__":
    main()
