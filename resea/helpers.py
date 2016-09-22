import yaml
import jinja2
from termcolor import cprint, colored


def error(msg):
    """Prints an error message and terminate the program."""
    sys.exit(colored('Error: {}'.format(msg), 'red'))


def notice(msg):
    """Prints an notice."""
    cprint('{}'.format(msg), 'yellow')


def info(msg):
    """Prints an informational message."""
    cprint(msg, 'blue')


def fmt(tmpl, vars):
    """Renders a template by jinja2."""
    return jinja2.Environment(
        trim_blocks=True,
        lstrip_blocks=True).from_string(tmpl).render(vars)


def loads_yaml(s, validator=None, path=None):
    """Loads a yaml string."""
    yml = yaml.safe_load(s)
    if validator is not None:
        try:
            yml = validator(yml)
        except ValidationError as e:
            if path is None:
                error("validation error: {}".format(str(e)))
            else:
                error("validation error in '{}': {}".format(path, str(e)))

    return yml


def load_yaml(path, **kwargs):
    """Loads a yaml file."""
    return loads_yaml(open(path).read(), path=path, **kwargs)
