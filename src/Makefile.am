lib_LTLIBRARIES = libwhale.la

# library version
libwhale_la_LDFLAGS=$(LTLDFLAGS)

libwhale_la_CPPFLAGS = -I$(top_srcdir)/include
######################################
# Cflags and Libs
libwhale_la_CXXFLAGS = $(LIBZ_CFLAGS) $(LIBICONV_CFLAGS) $(LIBEVENT_CFLAGS) $(HIREDIS_CFLAGS) $(MYSQLCPPCONN_CFLAGS) $(OCILIB_CFLAGS) $(CRYPTOPP_CFLAGS) $(MONGOC_CFLAGS) $(LIBBSON_CFLAGS) $(SQLITE3_CFLAGS) $(ODBC_CFLAGS) -std=c++11
libwhale_la_LIBADD = $(LIBEVENT_LIBS) $(HIREDIS_LIBS) $(MYSQLCPPCONN_LIBS) $(OCILIB_LIBS) $(LIBPQXX_LIBS) $(LIBBSON_LIBS) $(MONGOC_LIBS) $(SQLITE3_LIBS) $(ODBC_LIBS) $(LIBZ_LIBS) $(LIBICONV_LIBS) -lpthread

UUID_SRC =					\
huuid.cpp

if BUILD_LIBEVENT
LIBEVENT_SRC =					\
htcpserver.cpp					\
hhttpserver.cpp
endif

if BUILD_HIREDIS
HIREDIS_SRC =					\
hredis.cpp					\
hredisdataset.cpp
endif

if BUILD_OCILIB
OCILIB_SRC =					\
horacle.cpp					\
horacledataset.cpp
endif

if BUILD_MYSQLCPPCONN
MYSQLCPPCONN_SRC =				\
hmysql.cpp					\
hmysqldataset.cpp
endif

if BUILD_MONGOC
MONGOC_SRC =					\
hmongo.cpp					\
hmongodataset.cpp
endif

if BUILD_SQLITE3
SQLITE3_SRC =					\
hsqlite.cpp					\
hsqlitedataset.cpp
endif

if BUILD_LIBPQXX
LIBPQXX_SRC =					\
hpostgresql.cpp					\
hpostgresqldataset.cpp
endif

if BUILD_ODBC
ODBC_SRC =					\
hconnection.cpp 
endif

libwhale_la_SOURCES =				\
hsharememory.cpp				\
hthreadpool.cpp					\
hthread.cpp					\
hlog.cpp					\
hloopbuffer.cpp					\
hos.cpp						\
hdatetime.cpp					\
hmssql.cpp					\
hmssqldataset.cpp				\
hsetting.cpp					\
hoptions.cpp					\
hstring.cpp					\
hfile.cpp					\
hdir.cpp					\
hjson.cpp					\
hxml.cpp					\
hrandom.cpp					\
hsocket.cpp					\
htcpclient.cpp					\
hftpclient.cpp					\
hftpparser.cpp					\
hftpsocket.cpp					\
hftpstatus.cpp					\
hftptypes.cpp					\
htcppacket.cpp					\
hhttpclient.cpp					\
hdns.cpp					\
hbcd.cpp					\
hcrc.cpp					\
haes.cpp					\
hmd5.cpp					\
hrsa.cpp					\
hsha.cpp					\
hdes.cpp					\
hbase64.cpp					\
hzip.cpp					\
$(UUID_SRC)					\
$(HIREDIS_SRC)					\
$(LIBEVENT_SRC)					\
$(OCILIB_SRC)					\
$(SQLITE3_SRC)					\
$(MONGOC_SRC)					\
$(LIBPQXX_SRC)					\
$(MYSQLCPPCONN_SRC)				\
$(ODBC_SRC)
