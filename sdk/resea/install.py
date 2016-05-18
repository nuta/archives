import platform
import subprocess
from resea.helpers import plan, error, import_module


def install_os_requirements(os_requirements):
    os = {
        'Darwin': 'osx',
        'Linux':  'ubuntu'
    }.get(platform.system(), 'unknown')

    try:
        m = import_module('resea.os.{}'.format(os))
    except ImportError:
        return

    m.install(os_requirements.get(os, {}))

