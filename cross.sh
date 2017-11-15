#!/bin/bash
# This script will create a `fakeroot' direcotry under current dir
# finally, put TARGZ package to the UPDATEDIR

# Modify below information
NAME=libwhale
DIR=.

# TianQian TQ cross-compile
sysroot_usr_dir=/arm/tianqian/sysroot/usr
cross_compile_prefix=arm-none-linux-gnueabi
platform=armv5

TOPDIR=$(pwd)
major_version=1.0.0

ac_init_help=
ac_init_version=false
ac_init_env=
ac_init_clean=false

case "$1" in
    -help | --help | -h)
	ac_init_help=long ;;
    -version | --version | --versio | --vers | -V)
	ac_init_version=true ;;
    pi | nano | nanopi | orangepi)
	ac_init_env=orangepi ;;
    cl | clean)
	ac_init_clean=true ;;
esac

if test "$ac_init_help" = "long"; then
    cat <<EOF
Usage: $0 [OPTION]...

Configuration:
  -h, --help          display this help and exit
  -V, --version       display version information and exit
  pi, orangepi        using OrangePi cross-compiler to compile
  cl, clean           make distclean fisrt and remove fakeroot dir
EOF
fi

test -n "$ac_init_help" && exit 0
if $ac_init_version; then
    cat <<EOF
$major_version
EOF
    exit
fi

if test "$ac_init_env" = "orangepi"; then
    sysroot_usr_dir=/arm/nanopi/sysroot/usr
    cross_compile_prefix=arm-cortexa9-linux-gnueabihf
    platform=armhf
fi

export PKG_CONFIG_PATH=${sysroot_usr_dir}/lib/pkgconfig

if $ac_init_clean; then
    make distclean
    rm -rf fakeroot
    exit
fi

if [ ! -f Makefile ] ; then
    $DIR/configure						\
	--prefix=/usr						\
	--host=$cross_compile_prefix --build=x86_64-gnu-linux	\
	--with-libevent						\
	--with-sqlite3						\
	--with-libiconv
fi

mkdir -p fakeroot
make -j4 && make install-strip DESTDIR=$TOPDIR/fakeroot
[ $? -ne 0 ] && exit 1

# tar a package with version info
VERSION=$(grep "define VERSION" $DIR/config.h | sed 's/"$//' | sed 's/^.*"//')
TARGZ=${NAME}_${VERSION}-${platform}.tar.gz

echo "Entering directory '$TOPDIR/fakeroot'"
cd $TOPDIR/fakeroot

echo "Taring $TARGZ"
tar -czvf $TOPDIR/$TARGZ usr

echo "Exporting to ${sysroot_usr_dir}"
sed -i "s|prefix=/usr|prefix=${sysroot_usr_dir}|g" usr/lib/pkgconfig/libwhale.pc
cd usr
tar -czvf /tmp/$TARGZ include lib
tar -xvf /tmp/$TARGZ -C ${sysroot_usr_dir}
