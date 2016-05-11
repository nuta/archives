import os
import sys
import glob
import tempfile
import subprocess
import webbrowser
from resea.helpers import info, notice, plan, progress, load_yaml, render
from resea.validators import validate_package_yml
from resea.commands.clean import main as clean_main
from resea.commands.test import main as test_main


INDEX_HTML = """\
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Resea Doctor</title>
</head>
<body>
<h1>Resea Doctor</h1>
<hr>
<ul>
  <li><a href="file:///{{ csa_dir }}/index.html">Static code analysis</a></li>
  <li><a href="file:///{{ coverage_dir }}/index.html">Test coverage</a></li>
</ul>
</body>
</html>
"""

def main(argv_):
    clean_main([])

    plan('Validate the package')
    progress('check for the existence of README.md')
    if not os.path.exists('README.md'):
        notice('README.md not found')

    progress('validate package.yml')
    try:
        yml = load_yaml('package.yml', validator=validate_package_yml)
    except FileNotFoundError:
        sys.exit("'package.yml' not found")

    if yml['category'] in ['application', 'library']:
        tmp_dir = tempfile.mkdtemp(prefix='resea-doctor-')

        # build with Clang Static Analyzer and test with gcov
        # TODO: move it into the cpp package
        csa_dir = os.path.join(tmp_dir, 'csa')
        test_main([
           'HAL=posix_host',
           'MAKE=scan-build -o {} make'.format(csa_dir),
           'CFLAGS=-fprofile-arcs -ftest-coverage',
           'CXXFLAGS=-fprofile-arcs -ftest-coverage',
           'HAL_LINK=clang -fprofile-arcs -ftest-coverage -pthread -o'
        ])

        # collect coverages
        progress('Collecting coverages')
        coverage_info = os.path.join(tmp_dir, 'coverage.info')
        coverage_dir = os.path.join(tmp_dir, 'coverage')
        subprocess.run(['lcov', '--capture', '--directory', '.',
            '--quiet', '--output-file', coverage_info],
            check=True)
        subprocess.run(['genhtml', coverage_info, '--quiet',
            '--output-directory', coverage_dir],
            check=True)

        # generate index.html
        progress('Generating index.html')
        csa_dir = glob.glob(os.path.join(csa_dir, '*'))[0]
        index_html_path = os.path.join(tmp_dir, 'index.html')
        with open(index_html_path, 'w') as f:
            f.write(render(INDEX_HTML, locals()))

        progress('Opening the results')
        webbrowser.open_new_tab('file://' + index_html_path)

        plan('Done all diagnosis. Check out the details in the web browser.')
