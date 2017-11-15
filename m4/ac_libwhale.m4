# SYNOPSIS
#
#   AC_LIBWHALE_VERSION([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the redis library of a particular version (or newer)
#
#   This macro takes only one optional argument, required version of redis
#   3 library. If required version is not passed.
#
#   If no intallation prefix to the installed redis library is given the
#   macro searches under /usr, /usr/local, and /opt.
#
#   This macro calls:
#
#     AC_SUBST(LT_RELEASE)
#  	  AC_SUBST(LT_CURRENT)
#  	  AC_SUBST(LT_REVISION)
#	  AC_SUBST(LT_AGE)
#
#   And sets:
#
#     AC_LIBWHALE_VERSION
#
# LICENSE
#
#   Copyright (c) 2016 Liap <lianpingdevel@163.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AC_DEFUN([AC_LIBWHALE_VERSION],
[
AC_MSG_CHECKING([for libwhale version])

  LIBWHALE_VER=`cat VERSION`

  LT_MAJOR=`echo $LIBWHALE_VER | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  LT_AGE=`echo $LIBWHALE_VER | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  LT_REVISION=`echo $LIBWHALE_VER | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  LT_CURRENT=`expr $LT_MAJOR + $LT_AGE`

  AC_MSG_RESULT([v$LIBWHALE_VER ])

  AC_SUBST(LT_RELEASE)
  AC_SUBST(LT_CURRENT)
  AC_SUBST(LT_REVISION)
  AC_SUBST(LT_AGE)
])
