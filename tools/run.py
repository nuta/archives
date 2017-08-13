import argparse
import os
import subprocess
import sys
import shlex
import time

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("cmd")
    args = parser.parse_args()

    p = subprocess.Popen(shlex.split(args.cmd), preexec_fn=os.setsid,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)

    deansi = False
    line = ""
    while True:
        data = p.stdout.read(1)

        if len(data) == 0:
            time.sleep(0.5)
            if p.poll() is not None:
                return
        elif deansi:
            # TODO: fix this dirty hack
            if data == b"H":
                deansi = False
        elif data == b"\x1b":
            deansi = True
        else:
            s = data.decode("ascii")
            print(s, end="", flush=True)

            line += s
            if "\n" in s:
                if line.startswith("panic: "):
                    p.kill()
                    p.wait()
                    return
                line = ""

if __name__ == "__main__":
    main()
