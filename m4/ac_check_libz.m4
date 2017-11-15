# SYNOPSIS
#
#   AC_CHECK_LIBZ([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the libz library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of libz
#   library. If required version is not passed.
#
#   If no intallation prefix to the installed libz library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LIBZ_CFLAGS)
#     AC_SUBST(LIBZ_LIBS)
#
#   And sets:
#
#     HAVE_LIBZ
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_LIBZ],
[
    AC_ARG_WITH([libz],
        AS_HELP_STRING(
            [--with-libz=@<:@ARG@:>@],
            [use libz library @<:@default=yes@:>@, optionally specify the prefix for libz library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_LIBZ="no"
        elif test "$withval" = "yes"; then
            WANT_LIBZ="yes"
            ac_libz_path=""
        else
            WANT_LIBZ="yes"
            ac_libz_path="$withval"
        fi
        ],
        [WANT_LIBZ="no"]
    )

    LIBZ_CFLAGS=""
    LIBZ_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_LIBZ], [test "x$WANT_LIBZ" == "xyes"])

    if test "x$WANT_LIBZ" = "xyes"; then
        # Set our flags if we are checking a specific directory
        if test -n "$ac_libz_path" ; then
			if test -d "$ac_libz_path/include" ; then
				LIBZ_CFLAGS="-I$ac_libz_path/include"
			else
				LIBZ_CFLAGS="-I$ac_libz_path"
			fi

			if test -d "$ac_libz_path/lib" ; then
				LIBZ_LIBS="-L$ac_libz_path/lib -lz"
			else
				LIBZ_LIBS="-L$ac_libz_path -lz"
			fi
				
			SUCCESS=yes       
		else
			# if pkg-config is installed and libz has installed a .pc file,
			# then use the information of libz.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				LIBZ_CFLAGS=`$AX_PKG_CONFIG --cflags z 2>/dev/null`
				LIBZ_LIBS=`$AX_PKG_CONFIG --libs z 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
			
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(z, close, 
				[LIBZ_LIBS="-lz"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for libz library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			LIBZ_CFLAGS=""
			LIBZ_LIBS=""
			AC_MSG_ERROR([*** library libz is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_LIBZ], [], [Have the libz library])			
		fi
		
		AC_SUBST(LIBZ_CFLAGS)
		AC_SUBST(LIBZ_LIBS)
    fi
])
