# SYNOPSIS
#
#   AC_CHECK_ODBC([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the ocilib library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of ocilib
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed ocilib library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(ODBC_CFLAGS)
#     AC_SUBST(ODBC_LIBS)
#
#   And sets:
#
#     HAVE_ODBC
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_ODBC],
[
    AC_ARG_WITH([odbc],
        AS_HELP_STRING(
            [--with-odbc=@<:@ARG@:>@],
            [use odbc library @<:@default=yes@:>@, optionally specify the prefix for odbc library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_ODBC="no"
        elif test "$withval" = "yes"; then
            WANT_ODBC="yes"
            ac_odbc_path=""
        else
            WANT_ODBC="yes"
            ac_odbc_path="$withval"
        fi
        ],
        [WANT_ODBC="no"]
    )

    ODBC_CFLAGS=""
    ODBC_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_ODBC], [test "x$WANT_ODBC" == "xyes"])

	if test "x$WANT_ODBC" = "xyes"; then
	# Set our flags if we are checking a specific directory
		if test -n "$ac_odbc_path" ; then
			if test -d "$ac_odbc_path/include" ; then
				ODBC_CFLAGS="-I$ac_odbc_path/include"
			else
				ODBC_CFLAGS="-I$ac_odbc_path"
			fi

			if test -d "$ac_odbc_path/lib" ; then
				ODBC_LIBS="-L$ac_odbc_path/lib -lodbc"
			else
				ODBC_LIBS="-L$ac_odbc_path -lodbc"
			fi
	
			SUCCESS=yes
		else
			# if pkg-config is installed and odbc has installed a .pc file,
			# then use the information of odbc.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				ODBC_CFLAGS=`$AX_PKG_CONFIG --cflags odbc 2>/dev/null`
				ODBC_LIBS=`$AX_PKG_CONFIG --libs odbc 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
	
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(odbc, ocisprintf, 
				[ODBC_LIBS="-lnnz11 -lclntsh -lodbc"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for odbc library])
		
	
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			ODBC_CFLAGS=""
			ODBC_LIBS=""
			AC_MSG_ERROR([*** library odbc is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_ODBC], [], [Have the odbc library])
		fi
	
		AC_SUBST(ODBC_CFLAGS)
		AC_SUBST(ODBC_LIBS)
	fi
])
