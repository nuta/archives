import reseasdk


def test_version(capsys):
    reseasdk.main(['version'])
    stdout = capsys.readouterr()[0]
    assert stdout == 'ReseaSDK version {}\n'.format(reseasdk.__version__)
