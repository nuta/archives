import datetime
import os
import sys
import glob
import shutil
import subprocess
import webbrowser
from resea.package import load_packages
from resea.helpers import info, notice, plan, progress, load_yaml, render
from resea.validators import validate_package_yml
import resea.commands.clean


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

def main(argv_):
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
        sys.exit("'package.yml' not found")

    config, _ = load_packages(yml['depends'])
    lang = yml.get("lang")
    if lang is None:
        error("lang is not speicified in package.yml")

    # run lang's doctor
    lang_html_path = os.path.join(tmp_dir, 'lang.html')
    subprocess.run([config['LANG'][lang]['doctor'], lang_html_path,
        tmp_dir], check=True)

    # generate index.html
    progress('Generating index.html')
    with open(lang_html_path) as f:
        lang_html = f.read()

    index_html_path = os.path.join(tmp_dir, 'index.html')
    with open(index_html_path, 'w') as f:
        f.write(render(INDEX_HTML, {
                    'lang': lang_html,
                    'created_at': str(datetime.datetime.now())
                }))

    progress('Opening the results')
    webbrowser.open_new_tab('file://' + os.path.abspath(index_html_path))

    plan('Done all diagnosis. Check out the details in the web browser.')
