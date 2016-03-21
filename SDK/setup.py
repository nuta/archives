from setuptools import find_packages, setup

# XXX: read the ReseaSDK's version
exec(open("reseasdk/version.py").read())


setup_args = {
    'name': 'ReseaSDK',
    'version': __version__,
    'url': 'http://resea.net',
    'author': 'Seiya Nuta',
    'author_email': 'nuta@seiya.me',
    'description': 'A software development kit for Resea',
    'license': 'Public Domain',
    'packages': ['reseasdk', 'reseasdk.commands'],
    'scripts': ['bin/reseasdk'],
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
