#!/bin/bash
set -e

case $TARGET in
SDK)
    cd sdk
    make test
    ;;
resea.net)
    resea doctor-all /tmp/health
    resea docs-all   /tmp/docs
    ;;
*)
    cd $TARGET
    resea doctor
    ;;
esac
