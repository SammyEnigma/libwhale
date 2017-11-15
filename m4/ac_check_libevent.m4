# SYNOPSIS
#
#   AC_CHECK_LIBEVENT([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the libevent library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of libevent
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed libevent library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LIBEVENT_CFLAGS)
#     AC_SUBST(LIBEVENT_LIBS)
#
#   And sets:
#
#     HAVE_LIBEVENT
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianping_master@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_CHECK_LIBEVENT],
[
    AC_ARG_WITH([libevent],
        AS_HELP_STRING(
            [--with-libevent=@<:@ARG@:>@],
            [use libevent library @<:@default=yes@:>@, optionally specify the prefix for libevent library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_LIBEVENT="no"
        elif test "$withval" = "yes"; then
            WANT_LIBEVENT="yes"
            ac_libevent_path=""
        else
            WANT_LIBEVENT="yes"
            ac_libevent_path="$withval"
        fi
        ],
        [WANT_LIBEVENT="no"]
    )

    LIBEVENT_CFLAGS=""
    LIBEVENT_LIBS=""
    SUCCESS=no
	AM_CONDITIONAL([BUILD_LIBEVENT], [test "x$WANT_LIBEVENT" == "xyes"])

	if test "x$WANT_LIBEVENT" = "xyes"; then
		# Set our flags if we are checking a specific directory
		if test -n "$ac_libevent_path" ; then
			if test -d "$ac_libevent_path/include" ; then
				LIBEVENT_CFLAGS="-I$ac_libevent_path/include"
			else
				LIBEVENT_CFLAGS="-I$ac_libevent_path"
			fi

			if test -d "$ac_libevent_path/lib" ; then
				LIBEVENT_LIBS="-L$ac_libevent_path/lib -levent -levent_pthreads"
			else
				LIBEVENT_LIBS="-L$ac_libevent_path -levent -levent_pthreads"
			fi
	
			SUCCESS=yes
		else
			# if pkg-config is installed and libevent has installed a .pc file,
			# then use the information of event.pc
			AC_PATH_PROG([AX_PKG_CONFIG], [pkg-config])
			if test -n "$AX_PKG_CONFIG" ; then
				LIBEVENT_CFLAGS=`$AX_PKG_CONFIG --cflags libevent libevent_pthreads 2>/dev/null`
				LIBEVENT_LIBS=`$AX_PKG_CONFIG --libs libevent libevent_pthreads 2>/dev/null`
				test "x$?" = "x0" && SUCCESS=yes
			fi
	
			if test "x$SUCCESS" != "xyes" ; then
				AC_CHECK_LIB(event, event_base_new, 
				[LIBEVENT_LIBS="-levent -levent_pthreads"
				SUCCESS=yes])
			fi
		fi

		AC_MSG_CHECKING([for libevent library])
		
	
		if test "x$SUCCESS" != "xyes" ; then
			AC_MSG_RESULT([no])
			LIBEVENT_CFLAGS=""
			LIBEVENT_LIBS=""
			AC_MSG_ERROR([*** library libevent is required ***])
		else	
			AC_MSG_RESULT([yes])
			AC_DEFINE([HAVE_LIBEVENT], [], [Have the libevent library])
		fi
	
		AC_SUBST(LIBEVENT_CFLAGS)
		AC_SUBST(LIBEVENT_LIBS)
	fi
])
			
