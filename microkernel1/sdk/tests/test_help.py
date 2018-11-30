import resea


def test_help(capsys):
    resea.main(['help'])
    assert capsys.readouterr()[0].startswith('Usage: ')

    resea.main([])
    assert capsys.readouterr()[0].startswith('Usage: ')

    for command in resea.command_list:
        resea.main(['help', command])
        assert capsys.readouterr()[0].startswith('Usage: ')

