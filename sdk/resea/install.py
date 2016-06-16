import platform
from resea.helpers import import_module


def install_os_requirements(os_requirements):
    if platform.system() == 'Darwin':
        os = 'macos'
    elif platform.linux_distribution()[0] == 'Ubuntu':
        os = 'ubuntu'
    else:
        notice('unsupported platform -- use OS X or Ubuntu')

    m = import_module('resea.os.{}'.format(os))
    m.install(os_requirements.get(os, {}))

