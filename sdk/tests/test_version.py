import resea


def test_version(capsys):
    resea.main(['version'])
    stdout = capsys.readouterr()[0]
    assert stdout == 'ReseaSDK version {}\n'.format(resea.__version__)
