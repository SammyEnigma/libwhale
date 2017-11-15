# SYNOPSIS
#
#   AC_CHECK_LIBPQXX([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the pqxx library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of pqxx
#   library. If required version is not passed.
#
#   If no intallation prefix to the installed pqxx library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LIBPQXX_CFLAGS)
#     AC_SUBST(LIBPQXX_LIBS)
#
#   And sets:
#
#     HAVE_LIBPQXX
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_LIBPQXX],
[
    AC_ARG_WITH([pqxx],
        AS_HELP_STRING(
            [--with-pqxx=@<:@ARG@:>@],
            [use pqxx library @<:@default=yes@:>@, optionally specify the prefix for pqxx library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_LIBPQXX="no"
        elif test "$withval" = "yes"; then
            WANT_LIBPQXX="yes"
            ac_pqxx_path=""
        else
            WANT_LIBPQXX="yes"
            ac_pqxx_path="$withval"
        fi
        ],
        [WANT_LIBPQXX="no"]
    )

    LIBPQXX_CFLAGS=""
    LIBPQXX_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_LIBPQXX], [test "x$WANT_LIBPQXX" == "xyes"])

    if test "x$WANT_LIBPQXX" = "xyes"; then
        # Set our flags if we are checking a specific directory
        if test -n "$ac_pqxx_path" ; then
			if test -d "$ac_pqxx_path/include" ; then
				LIBPQXX_CFLAGS="-I$ac_pqxx_path/include"
			else
				LIBPQXX_CFLAGS="-I$ac_pqxx_path"
			fi

			if test -d "$ac_pqxx_path/lib" ; then
				LIBPQXX_LIBS="-L$ac_pqxx_path/lib -lpqxx"
			else
				LIBPQXX_LIBS="-L$ac_pqxx_path -lpqxx"
			fi
				
			SUCCESS=yes       
		else
			# if pkg-config is installed and pqxx has installed a .pc file,
			# then use the information of pqxx.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				LIBPQXX_CFLAGS=`$AX_PKG_CONFIG --cflags pqxx 2>/dev/null`
				LIBPQXX_LIBS=`$AX_PKG_CONFIG --libs pqxx 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
			
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(pqxx, close, 
				[LIBPQXX_LIBS="-lpqxx"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for pqxx library])		
		
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			LIBPQXX_CFLAGS=""
			LIBPQXX_LIBS=""
			AC_MSG_ERROR([*** library pqxx is required ***])
		else
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_LIBPQXX], [], [Have the pqxx library])			
		fi
		
		AC_SUBST(LIBPQXX_CFLAGS)
		AC_SUBST(LIBPQXX_LIBS)
    fi
])