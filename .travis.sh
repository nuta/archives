#!/bin/bash
set -e

case $TARGET in
SDK)
    cd sdk
    make test
    ;;
resea.net)
    resea doctor-all
    resea docs-all
    ;;
*)
    cd $TARGET
    resea doctor
    ;;
esac
