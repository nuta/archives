# 2fa
A command-line 2-factor authentication manager. This includes
[pyotp](https://github.com/pyotp/pyotp) and [pyqrcode](https://github.com/mnooner256/pyqrcode).

## Requirements
- Python 3
- gpg2

## Installation
```
$ git clone https://github.com/seiyanuta/2fa
$ cd 2fa
$ python3 setup.py install
```

## How to use

Register an account:
```
$ 2fa --register 'AWS (seiya)' '555555'
```

Generate codes:
```
$ 2fa
AWS (root)          281692      (23)
AWS (seiya)         628110      (23)
Dropbox             713351      (23)
GitHub              719876      (23)
Google              884966      (23)
```

Generate a QR code for Google Authenticator.
```
$ 2fa --qrcode "AWS (root)"
```

## Secret file
Token seeds are encrypted by gpg2 and saved as `~/.2fa.secrets`. Just copy the file to backup.

## License
2fa is released under MIT license and includes third-party libraries:
- `pyotp`: MIT License
- `pyqrcode`: BSD 3-clause License

