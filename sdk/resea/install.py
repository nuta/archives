import platform
import subprocess
from resea.helpers import plan, error, import_module


def install_os_requirements(os_requirements):
    if platform.system() == 'Darwin':
        os = 'osx'
    elif platform.linux_distribution()[0] == 'Ubuntu':
        os = 'ubuntu'
    else:
        notice('unsupported platform -- use OS X or Ubuntu')

    m = import_module('resea.os.{}'.format(os))
    m.install(os_requirements.get(os, {}))

