import platform
import subprocess
from resea.helpers import plan, error


def osx_install(requirements):
    def is_installed(package):
        p = subprocess.run(['brew', 'list', package],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.returncode == 0

    def install_package(package):
        try:
            subprocess.run(['brew', 'install', package], check=True)
        except subprocess.CalledProcessError:
            error('failed to install {}'.format(package))

    missing = [] 
    for package in requirements.get('homebrew', {}).get('packages', []):
        if not is_installed(package):
            missing.append(package)

    if missing != []:
        plan("Install requirements")
        for package in missing:
            install_package(package)


def ubuntu_install(requirements):
    pass # TODO


def install_os_requirements(requirements):
    os, install = {
        'Darwin': ('osx', osx_install),
        'Linux':  ('ubuntu', ubuntu_install),
    }.get(platform.system(), ('', None))

    if install is None:
        return

    install(requirements.get(os, {}))
