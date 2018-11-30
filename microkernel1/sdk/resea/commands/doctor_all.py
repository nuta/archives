import argparse
import os
import datetime
import glob
import shutil
from resea.commands.doctor import main as doctor_main
from resea.validators import validate_package_yml
from resea.helpers import load_yaml, plan, notice, render

SHORT_HELP = "generate all packages documentaion"
LONG_HELP = """
Usage: resea doctor-all
"""

INDEX_HTML = """\
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Resea Packages Health</title>
</head>
<body>

<h1>Resea Packages Health</h1>
<p>created at: {{ created_at }}</p>
<hr>

<ul>
{% for done, name in packages %}
    {% if done %}
        <li><a href="{{ name }}">{{ name }}</a></li>
    {% else %}
        <li>{{ name }} (an error ocurred during doctor)/li>
    {% endif %}
{% endfor %}
</ul>

</body>
</html>
"""

def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('outdir')
    args = parser.parse_args(argv)

    outdir = os.path.abspath(args.outdir)
    if os.path.exists(outdir):
        shutil.rmtree(outdir)

    os.makedirs(outdir)

    packages = []
    for package_yml_path in glob.glob('**/package.yaml', recursive=True):
        yml = load_yaml(package_yml_path, validator=validate_package_yml)
        package_name = yml['name']

        cwd = os.getcwd()
        os.chdir(os.path.dirname(package_yml_path))

        plan('Doctor {}'.format(package_name))

        try:
            doctor_main([])
        except:
            notice('failed to doctor {}'.format(package_name))
            packages.append((False, package_name))
        else:
            shutil.move('tmp/doctor', os.path.join(outdir, package_name))
            packages.append((True, package_name))

        os.chdir(cwd)

    index_html_path = os.path.join(outdir, 'index.html')
    with open(index_html_path, 'w') as f:
        f.write(render(INDEX_HTML, {
                    'packages': packages,
                    'created_at': str(datetime.datetime.now())
                }))
