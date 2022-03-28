# Security

> ![a funny xkcd comic about the security](https://imgs.xkcd.com/comics/security.png)
>
> [xkcd: Security](https://xkcd.com/538/)

## What MakeStack provides
- HTTPS

TODO: Add the supported ciphersuites list.

## What MakeStack does *NOT* provide
- **No device authentication**
    - **Be careful when you connect your local machine over public Wi-Fi!** Everyone is able to access your server APIs and impersonate a genuine device.
- No address space isolation
    - The kernel and the your app share same address space (strictly speaking, MMU is not available). **A bug in your app could lead to a serious vulnerabiity!**
- No flash encryption
    - **Don't embed sensitive data on the device!** It can be easily extracted from the device if an attacker has physical access!
- No application-level payload encryption
    - Payload (the firmware image, device log, etc.) is not encrypted.
    - Of course you can utilize transport-layer level secure communication protocols such as HTTPS, but be careful when you use serial ports or HTTP in a *public* Wi-Fi for development (`makestack dev`) !
- No firmware rollback (it's a *reliability* problem btw)
    - I'm looking forward to your Pull Request :)
- No secure boot
    - The firmware is easily modified if an attacker has physical access to the device.
