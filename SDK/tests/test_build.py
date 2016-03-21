import os
import yaml
import shutil
import pytest
import reseasdk


def test_build(package):
    build_dir = 'build/release'
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)

    reseasdk.main(['build', 'BUILTIN_APPS=kernel', 'HAL=posix_host'])
    assert os.path.exists('build/release/application')

    reseasdk.main(['clean'])
    assert not os.path.exists('build')
