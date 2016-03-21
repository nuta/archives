import shutil


SHORT_HELP = "remove generated files during builds"
LONG_HELP = """
Usage: reseasdk clean
"""


def clean():
    try:
        shutil.rmtree('build')
    except FileNotFoundError:
        pass


def main(args):
    clean()
