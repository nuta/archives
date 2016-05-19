import argparse
import os
import re
import sys
import jinja2
import resea
from resea.helpers import error, generating, exec_cmd


SHORT_HELP = "create a new package"
LONG_HELP = """
Usage: resea new package_name
"""


README_TEMPLATE = """\
# {{ package_name }}

"""

GITIGNORE_TEMPLATE = """\
/build
/packages
/tmp
"""

PACKAGE_YML_TEMPLATE = """\
name: {{ package_name }}
category:    # application, library or interface
# id:
license:     # MIT, BSD 2-clause, BSD 3-clause, Public Domain, GPLv2
summary:     # brief explanation
description: # longer explanation

warning:     # some important things to note
info:        # what we should know

author: {{ author }}
email: {{ email }}
homepage:    # e.g. http://example.com/foo/bar

# Prefered language
# lang: cpp

# Required packages
depends: []

# Interfaces which this package uses
uses: []

# Interfaces which this package implements
implements: []


#
#  Type definitions
#
types:


#
#  Interface definitions
#
interface:

#
#  Config
#
config:
"""

FILES = [
    # (filepath, template)
    ('README.md', README_TEMPLATE),
    ('.gitignore', GITIGNORE_TEMPLATE),
    ('package.yml', PACKAGE_YML_TEMPLATE),
    ('src/.gitkeep', ''),
    ('Documentation/.gitkeep', '')
]

PACKAGE_NAME_REGEX = '[a-z_][a-z0-9_]*'


def main(args_):
    parser = argparse.ArgumentParser(prog='resea new',
                                     description='create a new package')
    parser.add_argument('--git', action='store_true',
                        help='git init in the created directory')
    parser.add_argument('dir', help='directory')
    args = parser.parse_args(args_)
    package_name = os.path.basename(args.dir)

    if re.match(PACKAGE_NAME_REGEX, package_name) is None:
        error('The package name must be lowercase_with_underscore (regex: {})'
              .format(PACKAGE_NAME_REGEX))

    #
    #  generate the package directory
    #
    generating('MKDIR', package_name)
    try:
        os.mkdir(package_name)
    except FileExistsError:
        error("Directory already exists: '{}'".format(package_name))

    #
    #  Variables used in templates
    #
    author = exec_cmd('git config --get user.name',
                      ignore_failure=True).strip()
    email = exec_cmd('git config --get user.email',
                     ignore_failure=True).strip()

    #
    #  generate directories and files
    #
    for path, tmpl in FILES:
        f = '{}/{}'.format(package_name, path)
        dir = os.path.dirname(f)

        if not os.path.exists(dir):
            generating('MKDIR', dir)
            os.mkdir(dir)

        generating('GEN', f)
        open(f, 'w').write(jinja2.Template(tmpl).render(**locals()))

    # intialize as a new Git repository
    if args.git:
        exec_cmd('git init .', cwd=package_name)
