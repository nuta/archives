import platform
import subprocess
from resea.helpers import plan, error


def osx_install(requirements):
    def is_installed(args):
        package = list(filter(lambda x: not x.startswith('-'), args))[0]
        p = subprocess.run(['brew', 'list', package],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return p.returncode == 0

    def install_package(args):
        try:
            subprocess.run(['brew', 'install'] + args, check=True)
        except subprocess.CalledProcessError:
            error('failed to install {}'.format(args))

    missing = [] 
    for package in requirements.get('homebrew', {}).get('packages', []):
        args = package.split(' ')
        if not is_installed(args):
            missing.append(args)

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
