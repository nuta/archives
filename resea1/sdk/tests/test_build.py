import os
import shutil
import resea


def test_build(package):
    build_dir = 'build/release'
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)

    resea.main(['build', 'CXX=gcc', 'LINKFLAGS=', 'HAL=posix_host'])
    assert os.path.exists('build/release/application')

    resea.main(['clean'])
    assert not os.path.exists('build')
