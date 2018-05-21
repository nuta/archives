from setuptools import find_packages, setup

exec(open("resea/version.py").read())

setup_args = {
    'name': 'Resea',
    'version': __version__,
    'url': 'http://resea.net',
    'author': 'Seiya Nuta',
    'author_email': 'nuta@seiya.me',
    'description': 'A Resea developer kit',
    'license': 'Public Domain',
    'packages': find_packages(),
    'scripts': ['bin/resea'],
    'install_requires': open('requirements.txt').read().strip().splitlines(),
    'classifiers': [
        'Development Status :: 2 - Pre-Alpha',
        'Intended Audience :: Developers',
        'License :: Public Domain',
        'Programming Language :: Python :: 3'
    ],
}


if __name__ == '__main__':
    setup(**setup_args)
