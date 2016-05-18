import subprocess
from resea.helpers import error, plan


def update():
    try:
        subprocess.run(['sudo', 'apt-get', 'update', '-y'], check=True)
    except subprocess.CalledProcessError:
        error('failed to update the list of packages'.format(args))


def is_installed(package):
    p = subprocess.run(['dpkg', '-s', package],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return p.returncode == 0

def apt_install(args):
    try:
        subprocess.run(['sudo', 'apt-get', 'install', '-y'] + args, check=True)
    except subprocess.CalledProcessError:
        error('failed to install {}'.format(args))
       
def install(requirements):
    missing = [] 
    for package in requirements.get('apt', {}).get('packages', []):
        args = package.split(' ')
        package = list(filter(lambda x: not x.startswith('-'), args))[0]
        if not is_installed(package):
            missing.append(args)

    if missing != []:
        plan('Install requirements')
        update()
        for package in missing:
            apt_install(package)
