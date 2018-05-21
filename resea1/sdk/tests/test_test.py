import shutil
import resea


def test_test(package, capsys):
    build_dir = 'build/test'
    shutil.rmtree(build_dir, ignore_errors=True)

    resea.main(['test', 'CXX=gcc', 'LINKFLAGS=', 'HAL=posix_host'])

    assert 'tests passed' in capsys.readouterr()[0]

