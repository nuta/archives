import argparse
from glob import glob
import os
from resea.helpers import progress, load_yaml
from resea.docs import generate_package_doc, generate_package_index_doc, \
    generate_documentation_dir, generate_index_doc

SHORT_HELP = "generate docomentation"
LONG_HELP = """
Usage: resea alldocs --revision REVISION --outdir DIR
"""

def alldocs(args):
    packages = []
    # FIXME
    files = glob('package.yml') + glob('*/package.yml') + \
            glob('*/*/package.yml') + glob('*/*/*/package.yml') + \
            glob('*/*/*/*/package.yml') + glob('*/*/*/*/*/package.yml')

    kwargs = {
        'revision': args.revision
    }
    
    generate_documentation_dir('Documentation', os.path.join(args.outdir), **kwargs)

    for f in files:
        if '/packages/' in f:
            continue

        path = os.path.dirname(f)
        package = os.path.basename(path)
        progress('Generating documentaion: {}'.format(package))
        generate_package_doc(path, os.path.join(args.outdir, 'packages', package),
            **kwargs)
        packages.append({'name': package, 'summary': load_yaml(f)['summary'] })

    progress('Generating packages index')
    generate_package_index_doc(packages, os.path.join(args.outdir, 'packages', 'index.html'),
        **kwargs)

    progress('Generating documentation index')
    generate_index_doc(os.path.join(args.outdir, 'index.html'),
        **kwargs)

def main(args):
    parser = argparse.ArgumentParser(prog='resea docs',
                                     description='Generate HTML docs')
    parser.add_argument('--outdir', default="html_docs")
    parser.add_argument('--revision', default="unknown_revision")
    alldocs(parser.parse_args(args))
