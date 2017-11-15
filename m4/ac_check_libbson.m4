# SYNOPSIS
#
#   AC_CHECK_LIBBSON([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the bson library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of bson
#   library. If required version is not passed.
#
#   If no intallation prefix to the installed bson library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LIBBSON_CFLAGS)
#     AC_SUBST(LIBBSON_LIBS)
#
#   And sets:
#
#     HAVE_LIBBSON
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_LIBBSON],
[
    AC_ARG_WITH([libbson],
        AS_HELP_STRING(
            [--with-libbson=@<:@ARG@:>@],
            [use bson library @<:@default=yes@:>@, optionally specify the prefix for bson library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_LIBBSON="no"
        elif test "$withval" = "yes"; then
            WANT_LIBBSON="yes"
            ac_bson_path=""
        else
            WANT_LIBBSON="yes"
            ac_bson_path="$withval"
        fi
        ],
        [WANT_LIBBSON="no"]
    )

    LIBBSON_CFLAGS=""
    LIBBSON_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_LIBBSON], [test "x$WANT_LIBBSON" == "xyes"])

    if test "x$WANT_LIBBSON" = "xyes"; then
        # Set our flags if we are checking a specific directory
        if test -n "$ac_bson_path" ; then
			if test -d "$ac_bson_path/include" ; then
				LIBBSON_CFLAGS="-I$ac_bson_path/include"
			else
				LIBBSON_CFLAGS="-I$ac_bson_path"
			fi

			if test -d "$ac_bson_path/lib" ; then
				LIBBSON_LIBS="-L$ac_bson_path/lib -lbson-1.0"
			else
				LIBBSON_LIBS="-L$ac_bson_path -lbson-1.0"
			fi
				
			SUCCESS=yes       
		else
			# if pkg-config is installed and bson has installed a .pc file,
			# then use the information of bson.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				LIBBSON_CFLAGS=`$AX_PKG_CONFIG --cflags bson 2>/dev/null`
				LIBBSON_LIBS=`$AX_PKG_CONFIG --libs bson 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
			
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(bson, bcon_new, 
				[LIBBSON_LIBS="-lbson-1.0"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for bson library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			LIBBSON_CFLAGS=""
			LIBBSON_LIBS=""
			AC_MSG_ERROR([*** library bson is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_LIBBSON], [], [Have the bson library])			
		fi
		
		AC_SUBST(LIBBSON_CFLAGS)
		AC_SUBST(LIBBSON_LIBS)
    fi
])