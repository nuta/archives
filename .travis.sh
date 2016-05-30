#!/bin/bash
set -e

case $TARGET in
SDK)
    cd sdk
    make test
    ;;
*)
    cd $TARGET
    resea doctor
    ;;
esac
