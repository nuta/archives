import os


def test_new(package):
    assert os.path.exists('package.yml')
