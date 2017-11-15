# SYNOPSIS
#
#   AC_CHECK_HIREDIS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the hiredis library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of hiredis
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed hiredis library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(HIREDIS_CFLAGS)
#     AC_SUBST(HIREDIS_LIBS)
#
#   And sets:
#
#     HAVE_HIREDIS
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_HIREDIS],
[
    AC_ARG_WITH([hiredis],
        AS_HELP_STRING(
            [--with-hiredis=@<:@ARG@:>@],
            [use hiredis library @<:@default=yes@:>@, optionally specify the prefix for hiredis library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_HIREDIS="no"
        elif test "$withval" = "yes"; then
            WANT_HIREDIS="yes"
            ac_hiredis_path=""
        else
            WANT_HIREDIS="yes"
            ac_hiredis_path="$withval"
        fi
        ],
        [WANT_HIREDIS="no"]
    )

    HIREDIS_CFLAGS=""
    HIREDIS_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_HIREDIS], [test "x$WANT_HIREDIS" == "xyes"])

    if test "x$WANT_HIREDIS" = "xyes"; then
		# Set our flags if we are checking a specific directory
		if test -n "$ac_hiredis_path" ; then
			if test -d "$ac_hiredis_path/include" ; then
				HIREDIS_CFLAGS="-I$ac_hiredis_path/include"
			else
				HIREDIS_CFLAGS="-I$ac_hiredis_path"
			fi

			if test -d "$ac_hiredis_path/lib" ; then
				HIREDIS_LIBS="-L$ac_hiredis_path/lib -lhiredis"
			else
				HIREDIS_LIBS="-L$ac_hiredis_path -lhiredis"
			fi
	  
			SUCCESS=yes
		else
			# if pkg-config is installed and hiredis has installed a .pc file,
			# then use the information of hiredis.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				HIREDIS_CFLAGS=`$AX_PKG_CONFIG --cflags hiredis 2>/dev/null`
				HIREDIS_LIBS=`$AX_PKG_CONFIG --libs hiredis 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
		
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(hiredis, redisConnect, 
				[HIREDIS_LIBS="-lhiredis"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for hiredis library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			HIREDIS_CFLAGS=""
			HIREDIS_LIBS=""
			AC_MSG_ERROR([*** library hiredis is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_HIREDIS], [], [Have the hiredis library])
		fi
		
		AC_SUBST(HIREDIS_CFLAGS)
		AC_SUBST(HIREDIS_LIBS)
	fi
])
