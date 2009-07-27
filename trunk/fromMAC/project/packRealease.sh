#!/bin/bash

CONFIG_FILE="config.bat"
BUILD_PATH="build/Release-iphoneos/"
BUILD_NAME="ASPHALT4.app"

VERSION_MAJOR=`grep "^set VERSION_MAJOR=[0-9]" config.bat | sed -e 's/^.*=\([0-9]\).*$/\1/'`
VERSION_MINOR=`grep "^set VERSION_MINOR=[0-9]" config.bat | sed -e 's/^.*=\([0-9]\).*$/\1/'`
VERSION_BUILD=`grep "^set VERSION_BUILD=[0-9]" config.bat | sed -e 's/^.*=\([0-9]\).*$/\1/'`

VERSION=$VERSION_MAJOR$VERSION_MINOR$VERSION_BUILD

ARCH_NAME="Asphalt4_iPhone_iPodTouch_IGP_v${VERSION}_Final.zip"

sudo chmod -R 777 $BUILD_PATH$BUILD_NAME

CRT_DIR=`pwd`

cd $BUILD_PATH
zip -9ry $CRT_DIR/$ARCH_NAME $BUILD_NAME
cd $CRT_DIR

