import reseasdk


def test_help(capsys):
    reseasdk.main(['help'])
    assert capsys.readouterr()[0].startswith('Usage: ')

    reseasdk.main([])
    assert capsys.readouterr()[0].startswith('Usage: ')

    for command in reseasdk.command_list:
        reseasdk.main(['help', command])
        assert capsys.readouterr()[0].startswith('Usage: ')

