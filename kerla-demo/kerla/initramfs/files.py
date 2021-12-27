from . import Package
import os

FILES = {}
FILES["/dev/.keep"] = ""
FILES["/tmp/.keep"] = ""
FILES["/etc/banner"] = r"""
 _________________________________
< Rewrite in Rust ALL THE THINGS! >
 ---------------------------------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
""".lstrip()
FILES["/etc/resolv.conf"] = """
nameserver 1.1.1.1
""".lstrip()

FILES["/etc/passwd"] = """
root:x:0:0:root:/root:/bin/sh
""".lstrip()

FILES["/etc/group"] = """
root:x:0:
""".lstrip()

FILES["/root/GITHUB.txt"] = """
https://github.com/nuta/kerla
""".lstrip()

FILES["/root/PRIVACY.txt"] = """
PRIVACY

This SSH connection is NOT secure!
----------------------------------
TL;DR: Don't expect confidentiality. Consider it as an unencrypted
       protocol like HTTP.

In order not to change the fingerprint, the host key (/etc/dropbear_host_key)
is shared between all VMs and what's worse it can be read by anyone.

Because of that, an attacker can decrypt all messages by intercepting
the key exchange (so-called man-in-the-middle attack).

What I DO collect
-----------------
- The access time
- Your IP address and port number
- Kerla's kernel log (for debugging)

What I DON'T collect
--------------------
- Your SSH public key or password
- What you typed

Please note that while I'm not interested in who you are at all, I might
accidentally collect those information due to a Kerla's bug.

Don't type any identifiable or sensitive information!
"""

FILES["/etc/motd"] = """\
    \x1b[1;97;46m                                                                 \x1b[0m
    \x1b[1;97;46m                        Welcome to Kerla!                        \x1b[0m
    \x1b[1;97;46m                                                                 \x1b[0m

      We've just created a new ephemeral VM just for you. Have fun!

      - This microVM will be \x1b[4mstopped in few minutes\x1b[0m.

      - This SSH \x1b[4mconnection is NOT secure\x1b[0m since all VMs shares the
        single host key you can access. See PRIVACY.txt for details.

      - The Internet connection is disabled.

      Examples:

        # ls /bin
        # uname -a | head | grep Linux

      Type "exit" or "[Enter]~." to quit.
"""

FILES["/etc/dropbear_host_key"] = \
    b'\x00\x00\x00\x0bssh-ed25519\x00\x00\x00@/#_\x16\xe3\x97\xad,%\x84\x9c\x9a.\xd2\xe1\xb1\x003\xeb\xd5\xf4\xb2FL\xe7\x1d5~\x15N\xfc<\xf3\x95l\xa8\xc1;\x85\x92W\xd0\xc9\xe1j\x8f\xd9>W+\xa4\xee\xf8\x8b\x0e=dWf\r\xf8\xf2\xe0('


class Files(Package):
    def __init__(self):
        super().__init__()
        self.name = "files"
        self.version = ""
        self.url = None
        self.host_deps = []
        self.files = {path: path.lstrip("/") for path in FILES.keys()}

    def build(self):
        for path, content in FILES.items():
            self.add_file(path.lstrip("/"), content)
