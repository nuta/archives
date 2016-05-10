import os
import sys
import atexit
import datetime
import subprocess
from termcolor import cprint, colored
from resea.helpers import info, error, progress


def lprint(s):
    """Prints a log message beautifully."""

    # try to get the column length of terminal
    try:
        columns = int(subprocess.check_outptu('stty size', 'r').split()[1])
    except:
        columns = 80

    try:
        # log message format is '[package_name] TYPE: a message from the package'
        if not s.startswith("["):
            raise ValueError
        package_name, rest = s.split(' ', 1)
        package_name = package_name.lstrip('[').rstrip(']')
        type_, rest = rest.split(':', 1)
        type_ = type_.rstrip(':')
        body = rest.strip()
    except ValueError:
        # invalid log message format: just print it
        print(s)
    else:
        # valid log message
        if type_ == 'TEST':
            # test results
            if body in ['start', 'end']:
                return
            elif body.startswith("<pass>"):
                type_ = 'PASS'
            else:
                type_ = 'FAIL'

        # pick a color
        t = {
            'INFO':  (' I ', 'white', 'on_blue'),
            'BUG':   (' B ', 'white', 'on_red'),
            'PANIC': (' P ', 'white', 'on_red'),
            'WARN':  (' W ', 'white', 'on_yellow'),
            'DEBUG': (' D ', 'grey',  'on_white'),
            'PASS':  (' T ', 'white', 'on_green'),
            'FAIL':  (' T ', 'white', 'on_red'),
        }.get(type_, (' ',))

        pad_str = ' ' * (12 - len(package_name))
        package_name_str = colored(package_name, attrs=['bold'])
        type_str = colored(*t)
        print('{}{} {} '.format(pad_str, package_name_str, type_str), end='')

        column = 17  # the length of a string printed above
        for word in body.split(' '):
            if column + len(word) + 1 < columns:
                print(word + ' ', end='')
                column += len(word) + 1
            else:
                # line wrap
                print('\n{}{} '.format(' ' * 17, word, ' '), end='')
                column = 17 + len(word) + 1
        print('')

        if type_ == 'PANIC':
            # kernel panic: raise SystemExit to abort the emulator
            raise SystemExit


def try_parse(l):
    """
    Tries to TEST log messages. Returns 'pass' or 'fail' if `l` is a TEST log
    message or '' (an empty string) if it failed to parse.
    """
    try:
        test = l.split('TEST: ')[1]
        result = test.split(' ', 2)[0].lstrip('<').rstrip('>')
    except IndexError:
        result = ''
    return result


def atexit_handler(p):
    progress('Terminating the emulator')
    try:
        if p.poll() is not None:
            p.kill()
    except ProcessLookupError:
        pass


def run_emulator(cmd, test=False, env=None, save_log=None):
    if save_log is None:
        save_log = '/dev/null'
    if env is None:
        env = {}

    # prepend a header to the log file
    f = open(save_log, 'a')
    f.write('================ {} ================\n'.format(
            str(datetime.datetime.now())))

    # run test program speicified in HAL_RUN config
    p = subprocess.Popen(cmd, env=env,
                         stderr=subprocess.STDOUT,
                         stdout=subprocess.PIPE)

    atexit.register(atexit_handler, p)

    # parse log messages
    passed = 0
    failed = 0
    while True:
        l = p.stdout.readline().decode('utf-8').strip()

        if l == "" and p.poll() is not None:
            if test:
                error("the test program finished without 'TEST: end'")
            sys.exit(0)

        result = try_parse(l)
        if result == 'end':
            f.write(l + '\n')
            lprint(l)
            if test:
                if failed == 0:
                    cprint('ReseaSDK: All {} tests passed'.format(passed),
                           'green')
                else:
                    cprint('ReseaSDK: {} tests failed'.format(failed),
                           'red')
            p.terminate()
            p.kill()
            return
        elif result == 'pass':
            passed += 1
            f.write(l + '\n')
            lprint(l)
        elif result == 'fail':
            failed += 1
            f.write(l + '\n')
            lprint(l)
        else:
            f.write(l + '\n')
            try:
                lprint(l)
            except SystemExit:
                # kernel panic
                p.terminate()
                p.kill()
                return
