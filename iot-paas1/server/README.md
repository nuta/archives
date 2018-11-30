MakeStack Server
================

A MakeStack server backend.

How to set up development environment
--------------------------------------

```bash
$ brew install postgresql redis yarn
$ brew services list # make sure that postgresql and redis are running

$ bundler install --path vendor/bundle
$ rails db:migrate
$ pushd ../ui && yarn && popd
$ foreman start
```
