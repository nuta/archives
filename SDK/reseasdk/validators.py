import re


class ValidationError(Exception):
    pass


def validate_package_yml(d):
    try:
        desc = 'it should be a dict'
        assert isinstance(d, dict)

        toplevel_keys = [
            'name',
            'category',
            'license',
            'description',
            'author',
            'email',
            'homepage',
            'info',
            'warning',
            'type',
            'interface',
            'requires',
            'implements',
            'uses',
        ]
        for x in toplevel_keys:
            desc = "it should have '{}'".format(x)
            assert x in d

        desc = 'name should matches /^[a-zA-Z][a-z0-9_]*$/'
        assert re.match('^[a-zA-Z][a-zA-Z0-9_]*$', d['name'])

        for x in ['requires', 'implements']:
            desc = '{} should be a list'.format(x)
            assert isinstance(d[x], list)

        cats = ['application', 'library', 'interface', 'group']
        desc = 'category should be one of the following: {}'.format(
            ', '.join(cats))
        assert d['category'] in cats

        if d['interface'] is not None:
            desc = "'interface' should be a dict"
            assert isinstance(d['interface'], dict)

            for name, i in d['interface'].items():
                desc = '.{}: service name should matches ' \
                       '/^[a-z][a-zA-Z0-9_]*$/'.format(name)
                assert re.match('^[a-z][a-zA-Z0-9_]*$', name)

                desc = ".{}: service should have 'payloads'" \
                       "and 'id'".format(name)
                assert 'payloads' in i and 'id' in i

                desc = '.{}: service ID should an int and it should be' \
                       'larger than 0'.format(name)
                assert isinstance(i['id'], int) and i['id'] > 0

                desc = ".{}: 'payloads' should be a list or None".format(name)
                assert isinstance(i['payloads'], list) or i['payloads'] is None
                if i['payloads'] is not None:
                   for x in i['payloads']:
                       desc = ".{}: interface should have" \
                              "'name' and 'type'".format(name)
                       assert 'name' in x and 'type' in x

                       desc = '.{}({}): payload name should matches ' \
                              '/^[a-z][a-zA-Z0-9_]*$/'.format(
                                  name, x['name'])
                       assert re.match('^[a-z][a-zA-Z0-9_]*$', x['name'])

        if d['type'] is not None:
            desc = "'type' should be a dict"
            assert isinstance(d['type'], dict)

            for name, t in d['type'].items():
                desc = '.{}: interface name should matches ' \
                       '/^[a-z][a-zA-Z0-9_]*$/'.format(name)
                assert re.match('^[a-zA-Z][a-zA-Z0-9_]*$', name)

                desc = "{}: type should have 'type'".format(name)
                assert 'type' in t

                if t['type'] == 'const':
                    desc = "{}: type should have 'size'".format(name)
                    assert 'size' in t
                    desc = "{}: const type should have 'consts'".format(name)
                    assert 'consts' in t
                else:
                    desc = "{}: unknown type: '{}'".format(name, t['type'])
                    assert False

    except (KeyError, IndexError, AssertionError) as e:
        raise ValidationError(desc)

