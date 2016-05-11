import platform
import subprocess
from resea.helpers import plan, error


def osx_update():
    try:
        subprocess.run(['apt-get', 'update', '-y'], check=True)
    except subprocess.CalledProcessError:
        error('failed to update the list of packages'.format(args))


def osx_is_installed(package):
    p = subprocess.run(['brew', 'list', package],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.returncode == 0


def osx_install(args):
    try:
        subprocess.run(['brew', 'install'] + args, check=True)
    except subprocess.CalledProcessError:
        error('failed to install {}'.format(args))


def ubuntu_update():
    try:
        subprocess.run(['brew', 'update'], check=True)
    except subprocess.CalledProcessError:
        error('failed to update the list of packages'.format(args))


def ubuntu_is_installed(package):
    p = subprocess.run(['dpkg', '-s', package],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.returncode == 0


def ubuntu_install(args):
    try:
        subprocess.run(['apt-get', 'install', '-y'] + args, check=True)
    except subprocess.CalledProcessError:
        error('failed to install {}'.format(args))


def install_os_requirements(os_requirements):
    os, update, is_installed, install = {
        'Darwin': ('osx', osx_update, osx_is_installed, osx_install),
        'Linux':  ('ubuntu', ubuntu_update, ubuntu_is_installed, ubuntu_install),
    }.get(platform.system(), ('', None, None, None))

    if install is None:
        return

    requirements = os_requirements.get(os, {})
    missing = [] 
    for package in requirements.get('homebrew', {}).get('packages', []):
        args = package.split(' ')
        package = list(filter(lambda x: not x.startswith('-'), args))[0]
        if not is_installed(package):
            missing.append(args)

    if missing != []:
        plan('Install requirements')
        update()
        for package in missing:
            install(package)

