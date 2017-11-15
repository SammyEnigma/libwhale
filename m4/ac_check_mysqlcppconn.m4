# SYNOPSIS
#
#   AC_CHECK_MYSQLCPPCONN([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the mysqlcppconn library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of mysqlcppconn
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed mysqlcppconn library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(MYSQLCPPCONN_CFLAGS)
#     AC_SUBST(MYSQLCPPCONN_LIBS)
#
#   And sets:
#
#     HAVE_MYSQLCPPCONN
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_MYSQLCPPCONN],
[
    AC_ARG_WITH([mysqlcppconn],
        AS_HELP_STRING(
            [--with-mysqlcppconn=@<:@ARG@:>@],
            [use mysqlcppconn library @<:@default=yes@:>@, optionally specify the prefix for mysqlcppconn library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_MYSQLCPPCONN="no"
        elif test "$withval" = "yes"; then
            WANT_MYSQLCPPCONN="yes"
            ac_mysql_path=""
        else
            WANT_MYSQLCPPCONN="yes"
            ac_mysql_path="$withval"
        fi
        ],
        [WANT_MYSQLCPPCONN="no"]
    )

    MYSQLCPPCONN_CFLAGS=""
    MYSQLCPPCONN_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_MYSQLCPPCONN], [test "x$WANT_MYSQLCPPCONN" == "xyes"])

	if test "x$WANT_MYSQLCPPCONN" = "xyes"; then
		# Set our flags if we are checking a specific directory
		if test -n "$ac_mysql_path" ; then
			if test -d "$ac_mysql_path/include" ; then
				MYSQLCPPCONN_CFLAGS="-I$ac_mysql_path/include"
			else
				MYSQLCPPCONN_CFLAGS="-I$ac_mysql_path"
			fi

			if test -d "$ac_mysql_path/lib" ; then
				MYSQLCPPCONN_LIBS="-L$ac_mysql_path/lib -lmysqlcppconn"
			else
				MYSQLCPPCONN_LIBS="-L$ac_mysql_path -lmysqlcppconn"
			fi
	
			SUCCESS=yes
		else
			# if pkg-config is installed and mysqlcppconn has installed a .pc file,
			# then use the information of mysqlcppconn.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				MYSQLCPPCONN_CFLAGS=`$AX_PKG_CONFIG --cflags mysqlcppconn 2>/dev/null`
				MYSQLCPPCONN_LIBS=`$AX_PKG_CONFIG --libs mysqlcppconn 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
	
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(mysqlcppconn, mysql_close, 
				[MYSQLCPPCONN_LIBS="-lmysqlcppconn"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for mysqlcppconn library])
		
	
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			MYSQLCPPCONN_CFLAGS=""
			MYSQLCPPCONN_LIBS=""
			AC_MSG_ERROR([*** library mysqlcppconn is required ***])
		else	
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_MYSQLCPPCONN], [], [Have the mysqlcppconn library])
		fi
	
		AC_SUBST(MYSQLCPPCONN_CFLAGS)
		AC_SUBST(MYSQLCPPCONN_LIBS)
	fi
])
			