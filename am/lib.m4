# This file is part of GNU Dico
# Copyright (C) 2010, 2012 Sergey Poznyakoff
#
# GNU Dico is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Dico is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

dnl Arguments:
dnl   $1     --    Library to look for
dnl   $2     --    Function to check in the library
dnl   $3     --    Any additional libraries that might be needed
dnl   $4     --    Action to be taken when test succeeds
dnl   $5     --    Action to be taken when test fails
dnl   $6     --    Directories where the library may reside
AC_DEFUN([DICO_CHECK_LIB],
[m4_ifval([$4], , [AH_CHECK_LIB([$1])])dnl
AS_VAR_PUSHDEF([dico_lib], [mu_cv_lib_$1])dnl
AC_CACHE_CHECK([for $2 in -l$1], [dico_lib],
[AS_VAR_SET([dico_lib], [no])
 dico_check_lib_save_LIBS=$LIBS
 for path in "" $6
 do
   if test -n "$path"; then
     dico_ldflags="-L$path -l$1 $3"
   else
     dico_ldflags="-l$1 $3"
   fi
   LIBS="$dico_ldflags $mu_check_lib_save_LIBS"
   AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
                  [AS_VAR_SET([dico_lib], ["$mu_ldflags"])
		   break])
 done		  
 LIBS=$dico_check_lib_save_LIBS])
AS_IF([test "AS_VAR_GET([dico_lib])" != no],
      [m4_default([$4], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
  LIBS="-l$1 $LIBS"
])],
      [$5])dnl
AS_VAR_POPDEF([dico_lib])dnl
])# DICO_CHECK_LIB


