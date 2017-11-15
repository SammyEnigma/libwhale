# SYNOPSIS
#
#   AC_CHECK_MONGOC([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the mongoc library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of mongoc
#   library. If required version is not passed.
#
#   If no intallation prefix to the installed mongoc library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(MONGOC_CFLAGS)
#     AC_SUBST(MONGOC_LIBS)
#
#   And sets:
#
#     HAVE_MONGOC
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_MONGOC],
[
    AC_ARG_WITH([mongoc],
        AS_HELP_STRING(
            [--with-mongoc=@<:@ARG@:>@],
            [use mongoc library @<:@default=yes@:>@, optionally specify the prefix for mongoc library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_MONGOC="no"
        elif test "$withval" = "yes"; then
            WANT_MONGOC="yes"
            ac_mongoc_path=""
        else
            WANT_MONGOC="yes"
            ac_mongoc_path="$withval"
        fi
        ],
        [WANT_MONGOC="no"]
    )

    MONGOC_CFLAGS=""
    MONGOC_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_MONGOC], [test "x$WANT_MONGOC" == "xyes"])

    if test "x$WANT_MONGOC" = "xyes"; then
        # Set our flags if we are checking a specific directory
        if test -n "$ac_mongoc_path" ; then
			if test -d "$ac_mongoc_path/include" ; then
				MONGOC_CFLAGS="-I$ac_mongoc_path/include"
			else
				MONGOC_CFLAGS="-I$ac_mongoc_path"
			fi

			if test -d "$ac_mongoc_path/lib" ; then
				MONGOC_LIBS="-L$ac_mongoc_path/lib -lmongoc-1.0"
			else
				MONGOC_LIBS="-L$ac_mongoc_path -lmongoc-1.0"
			fi
				
			SUCCESS=yes       
		else
			# if pkg-config is installed and mongoc has installed a .pc file,
			# then use the information of mongoc.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				MONGOC_CFLAGS=`$AX_PKG_CONFIG --cflags mongoc 2>/dev/null`
				MONGOC_LIBS=`$AX_PKG_CONFIG --libs mongoc 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
			
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(mongoc, bcon_new, 
				[MONGOC_LIBS="-lmongoc-1.0"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for mongoc library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			MONGOC_CFLAGS=""
			MONGOC_LIBS=""
			AC_MSG_ERROR([*** library mongoc is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_MONGOC], [], [Have the mongoc library])			
		fi
		
		AC_SUBST(MONGOC_CFLAGS)
		AC_SUBST(MONGOC_LIBS)
    fi
])
