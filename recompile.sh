#!/bin/sh
# install - install a program, script, or datafile


svn up

CURRENT_VER=`cat VERSION`

LT_MAJOR=`echo $CURRENT_VER | sed 's/\([0-9]*\).\([0-9]*\).\([0-9]*\)/\1/'`
LT_AGE=`echo $CURRENT_VER | sed 's/\([0-9]*\).\([0-9]*\).\([0-9]*\)/\2/'`
LT_REVISION=`echo $CURRENT_VER | sed 's/\([0-9]*\).\([0-9]*\).\([0-9]*\)/\3/'`
#LT_REVISION=`expr $LT_REVISION + 1`

echo "${LT_MAJOR}.${LT_AGE}.${LT_REVISION}" > VERSION

#echo `cat VERSION`

autoreconf -if
# make distclean

if [ ! -f Makefile ] ; then
    ./configure --with-uuid=$(pwd)/deps/libuuid-2.23.2 --with-libevent=$(pwd)/deps/libevent-2.1.8-stable --with-mysqlcppconn=$(pwd)/deps/mysqlcppconn-1.1.7 --with-ocilib=$(pwd)/deps/ocilib-4.2.1 --with-hiredis=$(pwd)/deps/hiredis-3.0.0 --with-sqlite3=$(pwd)/deps/sqlite-3.11.1 --with-mongoc=$(pwd)/deps/mongo-c-driver --with-libbson=$(pwd)/deps/libbson-1.6.2
fi

make -j4
[ $? -ne 0 ] && exit 1

svn ci -m ""

make dist

VERSION=$(grep "define VERSION" config.h | sed 's/"$//' | sed 's/^.*"//')
echo "Current Version : $VERSION"


#svn ci -m "Commit new version: $VERSION"
