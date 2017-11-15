# SYNOPSIS
#
#   AC_CHECK_OCILIB([MINIMUM-VERSION])
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
#     AC_SUBST(OCILIB_CFLAGS)
#     AC_SUBST(OCILIB_LIBS)
#
#   And sets:
#
#     HAVE_OCILIB
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_OCILIB],
[
    AC_ARG_WITH([ocilib],
        AS_HELP_STRING(
            [--with-ocilib=@<:@ARG@:>@],
            [use ocilib library @<:@default=yes@:>@, optionally specify the prefix for ocilib library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_OCILIB="no"
        elif test "$withval" = "yes"; then
            WANT_OCILIB="yes"
            ac_ocilib_path=""
        else
            WANT_OCILIB="yes"
            ac_ocilib_path="$withval"
        fi
        ],
        [WANT_OCILIB="no"]
    )

    OCILIB_CFLAGS=""
    OCILIB_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_OCILIB], [test "x$WANT_OCILIB" == "xyes"])

	if test "x$WANT_OCILIB" = "xyes"; then
	# Set our flags if we are checking a specific directory
		if test -n "$ac_ocilib_path" ; then
			if test -d "$ac_ocilib_path/include" ; then
				OCILIB_CFLAGS="-I$ac_ocilib_path/include"
			else
				OCILIB_CFLAGS="-I$ac_ocilib_path"
			fi

			if test -d "$ac_ocilib_path/lib" ; then
				OCILIB_LIBS="-L$ac_ocilib_path/lib -lnnz11 -lclntsh -locilib"
			else
				OCILIB_LIBS="-L$ac_ocilib_path -lnnz11 -lclntsh -locilib"
			fi
	
			SUCCESS=yes
		else
			# if pkg-config is installed and ocilib has installed a .pc file,
			# then use the information of ocilib.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				OCILIB_CFLAGS=`$AX_PKG_CONFIG --cflags ocilib 2>/dev/null`
				OCILIB_LIBS=`$AX_PKG_CONFIG --libs ocilib 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
	
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(ocilib, ocisprintf, 
				[OCILIB_LIBS="-lnnz11 -lclntsh -locilib"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for ocilib library])
		
	
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			OCILIB_CFLAGS=""
			OCILIB_LIBS=""
			AC_MSG_ERROR([*** library ocilib is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_OCILIB], [], [Have the ocilib library])
		fi
	
		AC_SUBST(OCILIB_CFLAGS)
		AC_SUBST(OCILIB_LIBS)
	fi
])
