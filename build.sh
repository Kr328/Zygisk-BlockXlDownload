#!/bin/bash

cd "$(dirname "$0")" || exit 1

if [ ! -f "$ANDROID_NDK/ndk-build" ]; then
    echo "ANDROID_NDK enviromnent invalid"
    exit 1
fi

FILENAME="zygisk-block-xldownload.zip"

assert() {
    if ! "$@"; then
        echo "Run $* failed"
        exit 1
    fi
}

do_build() {
    NDK_PROJECT=. assert "$ANDROID_NDK/ndk-build"

    LIB_NAME="libxldownload.so"

    mkdir -p "zip"
    assert cp "module.prop" "zip/module.prop"

    mkdir -p "zip/zygisk"
    assert cp "libs/arm64-v8a/$LIB_NAME" "zip/zygisk/arm64-v8a.so"
    assert cp "libs/armeabi-v7a/$LIB_NAME" "zip/zygisk/armeabi-v7a.so"
    assert cp "libs/x86/$LIB_NAME" "zip/zygisk/x86.so"
    assert cp "libs/x86_64/$LIB_NAME" "zip/zygisk/x86_64.so"

    mkdir -p "zip/META-INF"
    assert cp -r META-INF/* "zip/META-INF/"

    pushd "zip" || exit 2
    assert zip -r "../$FILENAME" ./*
    popd || exit 2
}

do_clean() {
    rm -rf libs
    rm -rf obj
    rm -rf zip
    rm "$FILENAME"
}

case "$1" in
"build")
    do_build
    ;;
"clean")
    do_clean
    ;;
*)
    echo "Usage: ./build.sh <build|clean>"
    exit 1
    ;;
esac
