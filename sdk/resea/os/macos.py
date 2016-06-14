import subprocess
from resea.helpers import error, plan


def update():
    try:
        subprocess.run(['brew', 'update'], check=True)
    except subprocess.CalledProcessError:
        error('failed to update the list of packages'.format(args))


def is_tapped(tap):
    taps = subprocess.check_output(['brew', 'tap']) \
           .decode('utf-8') \
           .split('\n')
    return tap in taps


installed_packages = None
def is_installed(package):
    global installed_packages
    if installed_packages is None:
        out =  subprocess.run(['brew', 'list'],
                   stdout=subprocess.PIPE).stdout.decode('utf-8')
        installed_packages = out.strip().split('\n')

    return package.replace('.rb', '') in installed_packages


def brew_install(args):
    try:
        subprocess.run(['brew', 'install'] + args, check=True)
    except subprocess.CalledProcessError:
        error('failed to install {}'.format(args))


def brew_tap(tap):
    try:
        subprocess.run(['brew', 'tap', tap], check=True)
    except subprocess.CalledProcessError:
        error('failed to tap {}'.format(args))


def install(requirements):
    for tap in requirements.get('homebrew', {}).get('taps', []):
        if not is_tapped(tap):
            brew_tap(tap)

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
            brew_install(package)
