#!/usr/bin/env python3
import digitalocean
import requests
import os
import sys
import time
import logging

logger = logging.getLogger(__name__)
logging.basicConfig(format="[%(asctime)s] %(message)s", datefmt="%I:%M:%S", level=logging.INFO)

REPO = "nuta/github-actions-test"
LB_NAME = "seiya.me"
HEALTH_CHECK_URL = "https://seiya.me"

do_token = os.environ['DO_TOKEN']
manager = digitalocean.Manager(token=do_token)

def get_loadbalancer():
    for lb in manager.get_all_load_balancers():
        if lb.name == LB_NAME:
            return lb
    sys.exit("failed to locate the load balancer")

def deploy_vm(lb, vm):
    logger.info(f"Removing {vm.name} from the loadbalancer")
    lb.remove_droplets([vm.id])

    logger.info(f"Reboot {vm.name}")
    vm.reboot()

    logger.info(f"Sleep 30 seconds to wait for {vm.name} to be ready")
    time.sleep(30)

    # TODO: Check the health check status in the loadbalancer.

    logger.info(f"Add {vm.name} to the loadbalancer")
    lb.add_droplets([vm.id])

def main():
    new_release = requests.get(f"https://api.github.com/repos/{REPO}/releases/latest").json()
    new_release_id = new_release["id"]
    logger.info(f"Latest release: {new_release['name']} ({new_release_id})")

    vms = []
    for vm in manager.get_all_droplets():
        if "kerla" in vm.tags:
            vms.append(vm)

    lb = get_loadbalancer()
    for vm in vms:
        deploy_vm(lb, vm)

    # The load balancer is round-robin, so try # of vms times.
    logger.info(f"Helath checking...")
    for i in range(0, len(vms)):
        requests.get(HEALTH_CHECK_URL).raise_for_status()

    logger.info("successfully deployed")

if __name__ == "__main__":
    main()
