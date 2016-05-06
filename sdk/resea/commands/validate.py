import os
from resea.helpers import info, notice, plan, progress, load_yaml
from resea.validators import validate_package_yml


def main(argv_):
    plan('Validating the package')

    progress('check for the existence of README.md')
    if not os.path.exists('README.md'):
        notice('README.md not found')

    progress('validate package.yml')
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        notice("'package.yml' not found (are you in a package directory?)")
