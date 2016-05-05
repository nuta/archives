import os
import yaml
import shutil
import pytest
import resea


def test_test(package, capsys):
    build_dir = 'build/test'
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)

    with pytest.raises(SystemExit):
        resea.main(['test', 'BUILTIN_APPS=[kernel]', 'HAL=posix_host'])

    assert 'tests passed' in capsys.readouterr()[0]
