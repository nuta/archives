import pytest
import resea


def test_unknown_command(capsys):

    with pytest.raises(SystemExit) as e:
        resea.main(['QWERTYUIOP'])
        assert str(e) == 'command not found: {}\n'.format('QWERTYUIOP')
