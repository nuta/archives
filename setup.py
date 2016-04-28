from setuptools import setup, find_packages

setup(
    name='2fa',
    version='0.0.1',
    description='A command-line 2-factor authentication manager',
    long_description='',
    url='https://github.com/seiyanuta/2fa',
    scripts=['2fa'],
    install_requires=['pyqrcode'],
    packages=find_packages(),
    classifiers = [
        'Operating System :: POSIX',
        'Environment :: Console',
        'Topic :: Utilities'
    ]
)
