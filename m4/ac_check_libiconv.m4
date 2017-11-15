# SYNOPSIS
#
#   AC_CHECK_LIBICONV([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the libiconv library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of libiconv
#   library. If required version is not passed.
#
#   If no intallation prefix to the installed libiconv library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LIBICONV_CFLAGS)
#     AC_SUBST(LIBICONV_LIBS)
#
#   And sets:
#
#     HAVE_LIBICONV
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_LIBICONV],
[
    AC_ARG_WITH([libiconv],
        AS_HELP_STRING(
            [--with-libiconv=@<:@ARG@:>@],
            [use libiconv library @<:@default=yes@:>@, optionally specify the prefix for libiconv library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_LIBICONV="no"
        elif test "$withval" = "yes"; then
            WANT_LIBICONV="yes"
            ac_libiconv_path=""
        else
            WANT_LIBICONV="yes"
            ac_libiconv_path="$withval"
        fi
        ],
        [WANT_LIBICONV="no"]
    )

    LIBICONV_CFLAGS=""
    LIBICONV_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_LIBICONV], [test "x$WANT_LIBICONV" == "xyes"])

    if test "x$WANT_LIBICONV" = "xyes"; then
        # Set our flags if we are checking a specific directory
        if test -n "$ac_libiconv_path" ; then
			if test -d "$ac_libiconv_path/include" ; then
				LIBICONV_CFLAGS="-I$ac_libiconv_path/include"
			else
				LIBICONV_CFLAGS="-I$ac_libiconv_path"
			fi

			if test -d "$ac_libiconv_path/lib" ; then
				LIBICONV_LIBS="-L$ac_libiconv_path/lib -liconv"
			else
				LIBICONV_LIBS="-L$ac_libiconv_path -liconv"
			fi
				
			SUCCESS=yes       
		else
			# if pkg-config is installed and libiconv has installed a .pc file,
			# then use the information of iconv.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				LIBICONV_CFLAGS=`$AX_PKG_CONFIG --cflags iconv 2>/dev/null`
				LIBICONV_LIBS=`$AX_PKG_CONFIG --libs iconv 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
			
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(libiconv, close, 
				[LIBICONV_LIBS="-liconv"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for libiconv library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			LIBICONV_CFLAGS=""
			LIBICONV_LIBS=""
			AC_MSG_ERROR([*** library libiconv is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_LIBICONV], [], [Have the libiconv library])			
		fi
		
		AC_SUBST(LIBICONV_CFLAGS)
		AC_SUBST(LIBICONV_LIBS)
    fi
])
