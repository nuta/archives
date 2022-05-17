#!/usr/bin/env python3
import digitalocean
import os
import argparse
import time

do_token = os.environ['DO_TOKEN']
manager = digitalocean.Manager(token=do_token)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--slowly", action="store_true")
    args = parser.parse_args()

    for vm in manager.get_all_droplets():
        if "kerla" in vm.tags:
            print(f"Rebooting {vm.name}")
            vm.power_cycle()
            if args.slowly:
                print("Waiting for 300 seconds...")
                time.sleep(300)

if __name__ == "__main__":
    main()
