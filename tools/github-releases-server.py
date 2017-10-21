#!/usr/bin/env python3
import logging
import os
import tempfile
import json
import http.server
import shutil
import socketserver
import subprocess
import platform
import re
import glob
from distutils.spawn import find_executable

ARCHS = [
    # (arch, toolchain_prefix)
    ('x64', ''),
]


PORT = 8100

logger = logging.getLogger('github-releases-server')
logging.basicConfig(format='%(asctime)s %(levelname)s] %(message)s',
                    level=logging.INFO)

def get_os_image(device_type):
    if device_type == 'x86_64':
        return open(f'linux/build/{device_type}/bootfs/kernel7.img').read()
    elif device_type == 'raspberrypi3':
        return open(f'linux/build/{device_type}/bootfs/vmlinuz').read()
    elif device_type == 'mock':
        return b'mock kernel image!'
    else:
        raise f'unknown device type: `{device_type}\''

def generate_local_plugin(plugin_dir):
    with tempfile.TemporaryDirectory() as tmpdir:
        logger.info(plugin_dir)
        dest_dir = os.path.join(tmpdir, os.path.basename(plugin_dir))
        shutil.copytree(plugin_dir, dest_dir)
        cwd = os.getcwd()
        os.chdir(dest_dir)

        subprocess.run(['yarn'])

        if os.path.exists('binding.gyp'):
            for arch, toolchain_prefix in ARCHS:
                env = os.environ
                env.update({
                    'AR': f'{toolchain_prefix}ar',
                    'CC': f'{toolchain_prefix}gcc',
                    'CXX': f'{toolchain_prefix}g++',
                    'LINK': f'{toolchain_prefix}g++'
                })

                subprocess.Popen(['node-gyp', 'rebuild', '--arch', arch], env=env).wait()
                os.makedirs(os.path.join('native', arch))

                for nodefile in glob.glob('build/Release/*.node'):
                    shutil.copyfile(nodefile, os.path.join('native', arch, os.path.basename(nodefile)))

                shutil.rmtree('build')

            for unused_module in ['nan']:
                shutil.rmtree(os.path.join('node_modules', unused_module))

        shutil.make_archive('plugin', 'zip')
        zip = open('plugin.zip', 'rb').read()
        os.chdir(cwd)

        return zip

class Server (http.server.SimpleHTTPRequestHandler):
    def response_json(self, obj):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(bytes(json.dumps(obj), 'utf-8'))

    def response_bin(self, data):
        self.send_response(200)
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        # /repos/plugins/app-runtime.plugin.zip
        if self.path == '/repos/plugins/app-runtime.plugin.zip':
            appruntime_dir = os.path.normpath(os.path.join(os.path.dirname(__file__),
                                 '..', 'libs', 'app-runtime'))
            self.response_bin(generate_local_plugin(appruntime_dir))

        # /os/raspberrypi3.img
        m = re.match(r'^\/repos/os/(?P<device_type>[^.]+).img$', self.path)
        if m:
            self.response_bin(get_os_image(m.groupdict()['device_type']))

        # /repos/octocat/hdc1000/releases/latest
        m = re.match(r'^\/repos/(?P<repo>.+/.+)/releases/latest$', self.path)
        if m:
            host = self.headers['host']
            repo = m.groupdict()['repo']
            if repo == 'seiyanuta/makestack':
                # official plugins
                self.response_json({
                    'assets': [
                        {
                            'name': 'app-runtime.plugin.zip',
                            'browser_download_url': f'http://{host}/repos/plugins/app-runtime.plugin.zip'
                        }
                    ]
                })
            else:
                self.response_json({
                    'assets': [
                        {
                            'name': f'{os.path.basename(repo)}.plugin.zip',
                            'browser_download_url': f'http://{host}/repos/plugins/{repo}.zip'
                        }
                    ]
                })

logger.info('starting a server...')
with socketserver.TCPServer(("", PORT), Server) as httpd:
    logger.info(f'listening at {PORT}')
    httpd.serve_forever()
