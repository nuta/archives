# 2fa
A command-line 2-factor authentication manager. This includes
[pyotp](https://github.com/pyotp/pyotp) and [pyqrcode](https://github.com/mnooner256/pyqrcode).


## How to use

Register an account:
```
$ 2fa 'AWS (seiya)' '555555'
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

## License
2fa is released under MIT license and includes third-party libraries:
- `pyotp`: MIT License
- `pyqrcode`: BSD 3-clause License

