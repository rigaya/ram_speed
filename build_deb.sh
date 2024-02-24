#!/bin/sh

PACKAGE_NAME=ramspeed
PACKAGE_BIN=ramspeed
PACKAGE_OS=
PACKAGE_MAINTAINER=rigaya
PACKAGE_DEPENDS=
PACKAGE_DESCRIPTION=
PACKAGE_ROOT=.debpkg
PACKAGE_VERSION=`git describe --tags | cut -f 1 --delim="-"`
PACKAGE_ARCH=`uname -m`
PACKAGE_ARCH=`echo ${PACKAGE_ARCH} | sed -e 's/x86_64/amd64/g' | sed -e 's/aarch64/arm64/g'`

mkdir -p ${PACKAGE_ROOT}/DEBIAN
build_pkg/replace.py \
    -i build_pkg/template/DEBIAN/control \
    -o ${PACKAGE_ROOT}/DEBIAN/control \
    --pkg-name ${PACKAGE_NAME} \
    --pkg-bin ${PACKAGE_BIN} \
    --pkg-version ${PACKAGE_VERSION} \
    --pkg-arch ${PACKAGE_ARCH} \
    --pkg-maintainer ${PACKAGE_MAINTAINER} \
    --pkg-depends ${PACKAGE_DEPENDS} \
    --pkg-desc ${PACKAGE_DESCRIPTION}

mkdir -p ${PACKAGE_ROOT}/usr/bin
cp ramspeed ${PACKAGE_ROOT}/usr/bin
chmod +x ${PACKAGE_ROOT}/usr/bin/ramspeed

DEB_FILE="${PACKAGE_NAME}_${PACKAGE_VERSION}${PACKAGE_OS}_${PACKAGE_ARCH}.deb"
dpkg-deb -b "${PACKAGE_ROOT}" "${DEB_FILE}"