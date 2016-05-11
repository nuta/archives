import re


class ValidationError(Exception):
    pass


def validate_package_yml(d):
    try:
        desc = 'it should be a dict'
        assert isinstance(d, dict)

        for x in ['name', 'category', 'license', 'homepage']:
            desc = "'{}' is undefined".format(x)
            assert x in d

        desc = 'name should matches /^[a-z][a-z0-9_]*$/'
        assert re.match('^[a-z][a-z0-9_]*$', d['name'])

        for x in ['description', 'author', 'email', 'info', 'warning']:
            desc = '{} should be a string'.format(x)
            if x not in d or d[x] is None:
                d[x] = ""
            assert isinstance(d[x], str)

        for x in ['depends', 'implements', 'depends', 'implements',
                  'uses', 'conflicts']:
            if x not in d or d[x] is None:
                d[x] = []
            desc = '{} should be a list'.format(x)
            assert isinstance(d[x], list)

        for x in ['os_requirements', 'interface', 'types']:
            if x not in d or d[x] is None:
                d[x] = {}
            desc = '{} should be a dict'.format(x)
            assert isinstance(d[x], dict)

        # category
        cats = ['application', 'library', 'interface', 'group', 'misc']
        desc = 'category should be one of the following: {}'.format(
            ', '.join(cats))
        assert d['category'] in cats

        # interface
        for name, i in d['interface'].items():
            desc = '.{}: message type name should matches ' \
                   '/^[a-z][a-z0-9_]*$/'.format(name)
            assert re.match('^[a-z][a-z0-9_]*$', name)

            desc = ".{}: message type should have 'payloads'" \
                   "and 'id'".format(name)
            assert 'payloads' in i and 'id' in i

            desc = '.{}: message type ID should an int and it should be' \
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
                          '/^[a-z][a-z0-9_]*$/'.format(
                              name, x['name'])
                   assert re.match('^[a-z][a-z0-9_]*$', x['name'])

        # types
        for name, t in d['types'].items():
            desc = '.{}: type name should matches ' \
                   '/^[a-z][a-z0-9_]*$/'.format(name)
            assert re.match('^[a-z][a-z0-9_]*$', name)

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

        # config
        for x in ['config', 'global_config']:
            if isinstance(d.get(x), dict):
                d[x] = [d[x]]

        return d

    except (KeyError, IndexError, AssertionError) as e:
        raise ValidationError(desc)

