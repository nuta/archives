import pytest
import reseasdk


def test_unknown_command(capsys):

    with pytest.raises(SystemExit) as e:
        reseasdk.main(['QWERTYUIOP'])
        assert str(e) == 'command not found: {}\n'.format('QWERTYUIOP')
