import argparse
import datetime
import os
import sys
import glob
import shutil
import subprocess
import webbrowser
from resea.package import load_packages
from resea.helpers import info, error, success, notice, plan, progress, \
                          load_yaml, render
from resea.validators import validate_package_yml
from resea.var import get_var, expand_var
import resea.commands.clean

SHORT_HELP = "diagnosis the package"
LONG_HELP = """
Usage: resea doctor
"""

INDEX_HTML = """\
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Resea Doctor</title>
</head>
<body>
<h1>Resea Doctor Diagnosis</h1>
<p>{{ created_at }}</p>
<hr>
<ul>
{{ lang }}
</ul>
</body>
</html>
"""

def doctor():
    resea.commands.clean.main([])
    tmp_dir = os.path.join('tmp', 'doctor')
    shutil.rmtree(tmp_dir, ignore_errors=True)
    os.makedirs(tmp_dir)

    plan('Validate the package')
    progress('check for the existence of README.md')
    if not os.path.exists('README.md'):
        notice('README.md not found')

    progress('validate package.yml')
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found")

    if yml['category'] in ['application', 'library']:
        load_packages([yml['name']] + yml['depends'], {})
        lang = yml.get("lang")
        if lang is None:
            error("lang is not speicified in package.yml")

        # run lang's doctor
        lang_html_path = os.path.join(tmp_dir, 'lang.html')
        doctor = expand_var(get_var('LANGS')[lang]['doctor'])
        subprocess.Popen('{} {} {}'.format(doctor, lang_html_path,tmp_dir),
            shell=True).wait()

        # generate index.html
        progress('Generating index.html')
        with open(lang_html_path) as f:
            lang_html = f.read()
    else:
        lang_html = ''

    index_html_path = os.path.join(tmp_dir, 'index.html')
    with open(index_html_path, 'w') as f:
        f.write(render(INDEX_HTML, {
                    'lang': lang_html,
                    'created_at': str(datetime.datetime.now())
                }))

    return index_html_path


def main(argv_):
    parser = argparse.ArgumentParser(prog='resea doctor',
                                     description='diagnose the package')
    parser.add_argument('--open-browser', action='store_true',
        help='open results in a web browser')
    args = parser.parse_args(argv_)

    index_html_path = doctor()

    if args.open_browser:
        progress('Opening the results')
        webbrowser.open_new_tab('file://' + os.path.abspath(index_html_path))

    success('Done all diagnosis')

