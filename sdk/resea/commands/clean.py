import shutil


SHORT_HELP = "remove generated files"
LONG_HELP = """
Usage: resea clean
"""


def clean():
    try:
        shutil.rmtree('build')
    except FileNotFoundError:
        pass


def main(args):
    clean()
