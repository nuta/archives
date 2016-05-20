import argparse
import hashlib
import os
import sys
import subprocess
import time
import threading
import watchdog
import watchdog.events
import watchdog.observers
from resea.run import run_emulator
from resea.helpers import info, notice, error, plan
from resea.build import build, add_build_arguments
from resea.var import global_config, get_var

SHORT_HELP = "build and run tests"
LONG_HELP = """
Usage: resea test
"""


class FSEventHandler(watchdog.events.FileSystemEventHandler):

    def __init__(self):
        super().__init__()
        self.lock = threading.Lock()
        self.last_file = ''
        self.last_hash = ''

    def on_any_event(self, event):
        def run_test():
            argv = list(filter(lambda x: x != '-P', sys.argv))
            subprocess.Popen(argv).wait()
            info('ReseaSDK: watching changes (Ctrl-D to quit)')

        def is_target_ext(path):
            exts = [
                'c', 'h', 'S', 'cpp',
                'yml',
                'lds',
                'cfg',
                'sh', 'py'
            ]
            return os.path.splitext(path)[1].lstrip('.') in exts

        with self.lock:
            # ignore directory changes
            if not os.path.isdir(event.src_path):
                # XXX: exclude files in the build directory
                if '/build/test/' in event.src_path:
                    return

                try:
                    with open(event.src_path, 'rb') as f:
                        hash = hashlib.sha1(f.read()).hexdigest()
                except:
                    # the file may be deleted (e.g. Emacs backup files)
                    return

                path = os.path.basename(event.src_path)

                if (is_target_ext(path) and
                   (self.last_hash != hash or self.last_file != path)):
                    self.last_hash = hash
                    self.last_file = path
                    info('ReseaSDK: detected changes in files'
                         '-- starting a test')
                    threading.Thread(target=run_test, daemon=True).start()

def autotest(args):
    # look for .git
    cwd = os.getcwd()
    while os.getcwd() != '/':
        if os.path.exists('.git'):
            path = os.getcwd()
        os.chdir('..')
    if os.getcwd() != '/':
        path = os.getcwd()
    os.chdir(cwd)

    info('ReseaSDK: watching changes in {} (Ctrl-D to quit)'.format(path))
    observer = watchdog.observers.Observer()
    observer.schedule(FSEventHandler(), path, recursive=True)
    observer.daemon = True
    observer.start()
    try:
        sys.stdin.read(1)
    except KeyboardInterrupt:
        pass

    info('ReseaSDK: stopping')
    observer.stop()
    observer.join()


def test(args):
    global_config.set('TEST', True)
    global_config.set('ENV', 'test')
    build(args)

    plan('Invoking tests')
    cmd = [get_var('HAL_RUN'), get_var('BUILD_DIR') + '/application']
    return run_emulator(cmd, test=True, wait=args.wait)

def main(args_):
    parser = argparse.ArgumentParser(prog='resea test',
                                     description='test an executable')
    parser = add_build_arguments(parser)
    parser.add_argument('-P', action='store_true', help='Poll file changes and run tests automatically')
    parser.add_argument('--wait', action='store_true', help='wait for termination')
    args = parser.parse_args(args_)

    if args.P:
        autotest(args)
    else:
        return test(args)

