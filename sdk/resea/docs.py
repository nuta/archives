from glob import glob
import os
import markdown2
from resea.doc_templates import *
from resea.helpers import generating, load_yaml, render, error
from resea.validators import validate_package_yml


def md2html(path):
    return markdown2.markdown(open(path).read(),
               extras=['fenced-code-blocks', 'footnotes', 'tables'])


def generate_documentation_dir(indir, outdir, revision):
    for path in glob(os.path.join(indir, '*.md')):
        with open(os.path.join(outdir, 
            os.path.splitext(os.path.basename(path))[0] + '.html'), 'w') as f:
             f.write(render(DOC, {
                 'title': 'Resea Documentation',
                 'body': md2html(path),
                 'css': PACKAGE_DOC_CSS,
                 'revision': revision
             }))


def generate_package_doc(indir, outdir, revision):
    os.makedirs(outdir, exist_ok=True)

    try:
        yml = load_yaml(os.path.join(indir, 'package.yml'),
                   validator=validate_package_yml)
    except FileNotFoundError:
        error("'package.yml' not found")

    yml.update({
       'title': 'Resea Documentation - {}'.format(yml['name']),
       'css': PACKAGE_DOC_CSS,
       'readme': md2html(os.path.join(indir, 'README.md')),
       'revision': revision
    })

    generating('GENDOC', os.path.join(outdir, 'index.html'))
    with open(os.path.join(outdir, 'index.html'), 'w') as f:
        f.write(render(PACKAGE_DOC, yml))

    generate_documentation_dir(os.path.join(indir, 'Documentation'), outdir, revision)


def generate_package_index_doc(packages, out_path, revision):
    generating('GENDOC', out_path)
    with open(out_path, 'w') as f:
        f.write(render(PACKAGE_INDEX_DOC, {
            'title': 'Resea Documentation - Packages',
            'css': PACKAGE_INDEX_DOC_CSS,
            'packages': packages,
            'revision': revision
        }))


def generate_index_doc(out_path, revision):
    generating('GENDOC', out_path)
    with open(out_path, 'w') as f:
        f.write(render(INDEX_DOC, {
            'title': 'Resea Documentation',
            'css': INDEX_DOC_CSS,
            'revision': revision
        }))

