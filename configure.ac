#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libwhale], m4_esyscmd([tr -d '\n' < VERSION]), [lianpingdevel@163.com])
m4_pattern_allow([LT_MAJOR])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_LIBWHALE_VERSION

LTLDFLAGS="-version-info ${LT_CURRENT}:${LT_REVISION}:${LT_AGE}"
AC_SUBST(LTLDFLAGS)
 
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# Add bellow content if you need to link with another library
# PKG_CHECK_MODULES([HELLO], [libhello >= 1.0])
AC_CHECK_LIBEVENT
AC_CHECK_OCILIB
AC_CHECK_MYSQLCPPCONN
AC_CHECK_HIREDIS
AC_CHECK_SQLITE3
AC_CHECK_LIBPQXX
AC_CHECK_LIBBSON
AC_CHECK_MONGOC
AC_CHECK_ODBC
AC_CHECK_LIBZ
AC_CHECK_LIBICONV

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h mntent.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/socket.h sys/time.h sys/timeb.h syslog.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime ftime memset socket strrchr strtoul])

AC_CONFIG_FILES([Makefile
                 include/Makefile
		 include/rapidxml/Makefile
		 include/rapidjson/Makefile
		 include/whale/Makefile
                 src/Makefile])
AC_CONFIG_FILES([libwhale.pc])
AC_OUTPUT
