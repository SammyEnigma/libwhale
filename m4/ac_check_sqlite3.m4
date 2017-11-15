# SYNOPSIS
#
#   AC_CHECK_SQLITE3([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the sqlite3 library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of sqlite3
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed sqlite3 library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(SQLITE3_CFLAGS)
#     AC_SUBST(SQLITE3_LIBS)
#
#   And sets:
#
#     HAVE_SQLITE3
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_SQLITE3],
[
    AC_ARG_WITH([sqlite3],
        AS_HELP_STRING(
            [--with-sqlite3=@<:@ARG@:>@],
            [use sqlite3 library @<:@default=yes@:>@, optionally specify the prefix for sqlite3 library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_SQLITE3="no"
        elif test "$withval" = "yes"; then
            WANT_SQLITE3="yes"
            ac_sqlite3_path=""
        else
            WANT_SQLITE3="yes"
            ac_sqlite3_path="$withval"
        fi
        ],
        [WANT_SQLITE3="no"]
    )

    SQLITE3_CFLAGS=""
    SQLITE3_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_SQLITE3], [test "x$WANT_SQLITE3" == "xyes"])

	if test "x$WANT_SQLITE3" = "xyes"; then
	# Set our flags if we are checking a specific directory
		if test -n "$ac_sqlite3_path" ; then
			if test -d "$ac_sqlite3_path/include" ; then
				SQLITE3_CFLAGS="-I$ac_sqlite3_path/include"
			else
				SQLITE3_CFLAGS="-I$ac_sqlite3_path"
			fi

			if test -d "$ac_sqlite3_path/lib" ; then
				SQLITE3_LIBS="-L$ac_sqlite3_path/lib -lsqlite3"
			else
				SQLITE3_LIBS="-L$ac_sqlite3_path -lsqlite3"
			fi
	
			SUCCESS=yes
		else
			# if pkg-config is installed and sqlite3 has installed a .pc file,
			# then use the information of sqlite3.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				SQLITE3_CFLAGS=`$AX_PKG_CONFIG --cflags sqlite3 2>/dev/null`
				SQLITE3_LIBS=`$AX_PKG_CONFIG --libs sqlite3 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
	
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(sqlite3, sqlite3_close, 
				[SQLITE3_LIBS="-lsqlite3"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for sqlite3 library])
		
	
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			SQLITE3_CFLAGS=""
			SQLITE3_LIBS=""
			AC_MSG_ERROR([*** library sqlite3 is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_SQLITE3], [], [Have the sqlite3 library])
		fi
	
		AC_SUBST(SQLITE3_CFLAGS)
		AC_SUBST(SQLITE3_LIBS)
	fi
])
