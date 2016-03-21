from glob import glob
import os
import markdown
from reseasdk.doc_templates import *
from reseasdk.helpers import generating, load_yaml, render, error
from reseasdk.validators import validate_package_yml


def md2html(path):
    return markdown.markdown(open(path).read(),
               output_format='html5')


def generate_package_doc(indir, outdir):
    os.makedirs(outdir, exist_ok=True)

    try:
        yml = load_yaml(os.path.join(indir, 'package.yml'),
                   validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found")

    yml.update({
       'title': 'Resea Documentation - {}'.format(yml['name']),
       'css': PACKAGE_DOC_CSS,
       'readme': md2html(os.path.join(indir, 'README.md'))
    })

    generating('GENDOC', os.path.join(outdir, 'index.html'))
    with open(os.path.join(outdir, 'index.html'), 'w') as f:
        f.write(render(PACKAGE_DOC, yml))

    for path in glob('Documentation/*.md'):
        with open(os.path.join(outdir, os.path.basename(path)), 'w') as f:
             f.write(render(DOC, {
                 'title': 'Resea Documentation',
                 'body': md2html(path),
                 'css': PACKAGE_DOC_CSS
             }))


def generate_package_index_doc(packages, out_path):
    generating('GENDOC', out_path)
    with open(out_path, 'w') as f:
        f.write(render(PACKAGE_INDEX_DOC, {
            'title': 'Resea Documentation - Packages',
            'css': PACKAGE_INDEX_DOC_CSS,
            'packages': packages
        }))


def generate_index_doc(out_path):
    generating('GENDOC', out_path)
    with open(out_path, 'w') as f:
        f.write(render(INDEX_DOC, {
            'title': 'Resea Documentation',
            'css': INDEX_DOC_CSS,
        }))

