UUID_INCLUDE =					\
util/huuid.h

if BUILD_LIBEVENT
LIBEVENT_INCLUDE =				\
net/htcpserver.h				\
net/hhttpserver.h
endif

if BUILD_HIREDIS
HIREDIS_INCLUDE =				\
data/hredis.h					\
data/hredisdataset.h
endif

if BUILD_OCILIB
OCILIB_INCLUDE =				\
data/horacle.h					\
data/horacledataset.h
endif

if BUILD_MYSQLCPPCONN
MYSQLCPPCONN_INCLUDE =				\
data/hmysql.h					\
data/hmysqldataset.h
endif

if BUILD_SQLITE3
SQLITE3_INCLUDE =				\
data/hsqlite.h					\
data/hsqlitedataset.h
endif

if BUILD_MONGOC
MONGOC_INCLUDE =				\
data/hmongo.h					\
data/hmongodataset.h
endif

if BUILD_LIBPQXX
LIBPQXX_INCLUDE =				\
data/hpostgresql.h				\
data/hpostgresqldataset.h
endif

if BUILD_ODBC
ODBC_INCLUDE = 					\
data/hconnection.h
endif

basicdir=$(includedir)/whale/basic

basic_HEADERS =					\
basic/hbytearray.h				\
basic/hlisthandler.h				\
basic/hdatetime.h				\
basic/hevent.h					\
basic/heventpool.h				\
basic/herror.h					\
basic/hlog.h					\
basic/hloopbuffer.h				\
basic/hos.h					\
basic/hplatform.h				\
basic/hsharememory.h				\
basic/hthread.h					\
basic/hthreadpool.h				\
basic/hthreadsafelist.h				\
basic/hthreadsafepriorityqueue.h

cryptodir=$(includedir)/whale/crypto

crypto_HEADERS =				\
crypto/hcrc.h					\
crypto/hbcd.h					\
crypto/haes.h					\
crypto/hbase64.h				\
crypto/hmd5.h					\
crypto/hrsa.h					\
crypto/hdes.h					\
crypto/hzip.h					\
crypto/hsha.h

whaledatadir=$(includedir)/whale/data

whaledata_HEADERS =				\
data/hmssql.h					\
data/hmssqldataset.h				\
$(MYSQLCPPCONN_INCLUDE)				\
$(OCILIB_INCLUDE)				\
$(LIBPQXX_INCLUDE)				\
$(HIREDIS_INCLUDE)				\
$(SQLITE3_INCLUDE)				\
$(MONGOC_INCLUDE)				\
$(ODBC_INCLUDE)

netdir=$(includedir)/whale/net

net_HEADERS =					\
net/hdns.h					\
net/hftpclient.h				\
net/hftpparser.h				\
net/hftpsocket.h				\
net/hftpstatus.h				\
net/hftptypes.h					\
net/hsocket.h					\
net/htcpclient.h				\
net/htcppacket.h				\
net/hhttpclient.h				\
$(LIBEVENT_INCLUDE)

utildir=$(includedir)/whale/util

util_HEADERS =					\
util/hdir.h					\
util/hfile.h					\
util/hjson.h					\
util/hoptions.h					\
util/hsetting.h					\
util/hstring.h					\
util/hxml.h					\
util/hrandom.h					\
$(UUID_INCLUDE)
