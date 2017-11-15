#!/bin/bash

set -e

if [ -f Makefile ] ; then
    make distclean
    rm -rf fakeroot
fi

mkdir -p fakeroot
./configure --prefix=$(pwd)/fakeroot/usr/local --with-libevent=$(pwd)/deps/libevent-2.1.8-stable --with-mysqlcppconn=$(pwd)/deps/mysqlcppconn-1.1.7 --with-sqlite3=$(pwd)/deps/sqlite-3.11.1 --with-libiconv=$(pwd)/deps/libiconv-1.15 --with-libz 

make -j4 && make install
[ $? -ne 0 ] && exit 1

exit 0
