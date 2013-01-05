# This file is part of grecs - Gray's Extensible Configuration System -*- autoconf -*-
# Copyright (C) 2007, 2009-2012 Sergey Poznyakoff
#
# Grex is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# Grex is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Grex.  If not, see <http://www.gnu.org/licenses/>.

# _GRECS_MANGLE_OPTION(NAME)
# -------------------------
# Convert NAME to a valid m4 identifier, by replacing invalid characters
# with underscores, and prepend the _GRECS_OPTION_ suffix to it.
AC_DEFUN([_GRECS_MANGLE_OPTION],
[[_GRECS_OPTION_]m4_bpatsubst($1, [[^a-zA-Z0-9_]], [_])])

# _GRECS_SET_OPTION(NAME)
# ----------------------
# Set option NAME. 
AC_DEFUN([_GRECS_SET_OPTION],
[m4_define(_GRECS_MANGLE_OPTION([$1]), 1)])

# _GRECS_IF_OPTION_SET(NAME,IF-SET,IF-NOT-SET)
# -------------------------------------------
# Check if option NAME is set.
AC_DEFUN([_GRECS_IF_OPTION_SET],
[m4_ifset(_GRECS_MANGLE_OPTION([$1]),[$2],[$3])])

# _GRECS_OPTION_SWITCH(NAME1,IF-SET1,[NAME2,IF-SET2,[...]],[IF-NOT-SET])
# ------------------------------------------------------------------------
# If NAME1 is set, run IF-SET1.  Otherwise, if NAME2 is set, run IF-SET2.
# Continue the process for all name-if-set pairs within [...].  If none
# of the options is set, run IF-NOT-SET.
AC_DEFUN([_GRECS_OPTION_SWITCH],
[m4_if([$4],,[_GRECS_IF_OPTION_SET($@)],dnl
[$3],,[_GRECS_IF_OPTION_SET($@)],dnl
[_GRECS_IF_OPTION_SET([$1],[$2],[_GRECS_OPTION_SWITCH(m4_shift(m4_shift($@)))])])])

# _GRECS_SET_OPTIONS(OPTIONS)
# ----------------------------------
# OPTIONS is a space-separated list of Grecs options.
AC_DEFUN([_GRECS_SET_OPTIONS],
[m4_foreach_w([_GRECS_Option], [$1], [_GRECS_SET_OPTION(_GRECS_Option)])])

# GRECS_SETUP([dir],[OPTIONS],[pp-setup-file])
# dir - Directory in the source tree where grecs has been cloned.
# OPTIONS are:
#   no-preproc         Disable the use of preprocessor.
#   std-pp-setup       Install standard pp-setup file.
#   pp-setup-option    Add the --with-pp-setup-file option to the
#                      configuration file.  The option allows user to
#                      control whether the pp-setup file is installed.
#   tests              Build tests.
#   getopt             Add getopt.m4 to the distribution.
#   git2chg            Add git2chg.awk to the distribution.
#   syntax-doc         Add doc/grecs-syntax.texi to the distribution.
#   install            Build installable library.
#   shared             Build shared (convenience) library.
#   install-headers    [with "shared"] Install Grecs headers to
#                      GRECS_INCLUDE_DIR.
#
# The pp-setup-file argument supplies the pathname of the preprocessor
# setup file in the source tree.  It is ignored if std-pp-setup option is
# given.
#
# If neither std-pp-setup option, nor pp-setup-file argument are supplied,
# no preprocessor setup file is installed.

AC_DEFUN([GRECS_SETUP],[
  m4_pushdef([grecsdir],m4_if($1,[.],,$1,,[grecs/],$1/))
  AC_PROG_YACC
  AM_PROG_LEX

  AC_HEADER_SYS_WAIT

  AC_SUBST([GRECS_SUBDIR],m4_if($1,,grecs,$1))
  _GRECS_SET_OPTIONS([$2])
  # **********************	
  # Preprocessor
  # **********************
  _GRECS_IF_OPTION_SET([no-preproc],  
        [use_ext_pp=no],
        [AC_ARG_WITH([preprocessor],
		      AC_HELP_STRING([--without-preprocessor],
				     [do not use external preprocessor]),
	  [case "${withval}" in
	   yes) use_ext_pp=yes ;;
	   no)  use_ext_pp=no ;;
	   *)   AC_MSG_ERROR(bad value ${withval} for --with-preprocessor) ;;
	   esac],[use_ext_pp=yes])])

  if test $use_ext_pp != no; then
    # Check for default preprocessor
    AC_ARG_VAR([DEFAULT_PREPROCESSOR],
               [Set default preprocessor name])
    if test -z "$DEFAULT_PREPROCESSOR" ; then
      DEFAULT_PREPROCESSOR="m4 -s"
    fi
  
    save_PATH=$PATH
    PREPROC_OPTIONS=`echo $DEFAULT_PREPROCESSOR | sed -n 's/[[^ ]][[^ ]]* //p'`
    case "$DEFAULT_PREPROCESSOR" in
    /*) PATH=`expr $DEFAULT_PREPROCESSOR : '\(.*\)/.*'`:$PATH
        DEFAULT_PREPROCESSOR=`expr $DEFAULT_PREPROCESSOR : '.*/\(.*\)'`;;
    esac
    AC_PATH_PROG(PPBIN, $DEFAULT_PREPROCESSOR)
    DEFAULT_PREPROCESSOR=$PPBIN
    if test -n "$DEFAULT_PREPROCESSOR"; then
      DEFAULT_PREPROCESSOR="$DEFAULT_PREPROCESSOR $PREPROC_OPTIONS"
      _GRECS_IF_OPTION_SET([std-pp-setup],
        [PP_SETUP_FILE='pp-setup'],
	[m4_if([$3],,[PP_SETUP_FILE=],[PP_SETUP_FILE='$3'])])
      AC_SUBST(PP_SETUP_FILE)
      if test -n "$PP_SETUP_FILE"; then
        _GRECS_IF_OPTION_SET([pp-setup-option],
	  [AC_ARG_WITH([pp-setup-file],
		      AC_HELP_STRING([--with-pp-setup-file],
				     [install the default pp-setup file]),
	   [case "${withval}" in
	    yes) ;;
	    no)  PP_SETUP_FILE=;;
	    *)   AC_MSG_ERROR([bad value ${withval} for --with-pp-setup-file]) ;;
	    esac])],
	  [case $PPBIN in
           *m4) ;; # Install default pp-setup
	   *) PP_SETUP_FILE=;; # Skip it
           esac])
      fi
    fi
    PATH=$save_PATH
    DEFAULT_PREPROCESSOR="\\\"$DEFAULT_PREPROCESSOR\\\""
  else
    DEFAULT_PREPROCESSOR=NULL
  fi
  _GRECS_IF_OPTION_SET([tests],
                       [m4_pushdef([TESTDIR],grecsdir[tests])
		        AC_CONFIG_TESTDIR(TESTDIR)
		        AC_CONFIG_FILES(TESTDIR/Makefile TESTDIR/atlocal)
			m4_popdef([TESTDIR])
		        AM_MISSING_PROG([AUTOM4TE], [autom4te])
                        GRECS_TESTDIR=tests
		       ])
  _GRECS_IF_OPTION_SET([getopt],[
    AC_CHECK_HEADERS([getopt.h])
    AC_CHECK_FUNCS([sysconf getdtablesize getopt_long])
    GRECS_BUILD_AUX="build-aux/getopt.m4"
   ])
  _GRECS_IF_OPTION_SET([git2chg],[GRECS_BUILD_AUX="$GRECS_BUILD_AUX build-aux/git2chg.awk"])
  AM_CONDITIONAL([GRECS_COND_META1_PARSER],
                 _GRECS_OPTION_SWITCH([parser-meta1],[true],
		                      [all-parsers],[true],
				      [false]))
  AM_CONDITIONAL([GRECS_COND_BIND_PARSER],
                 _GRECS_OPTION_SWITCH([parser-bind],[true],
		                      [all-parsers],[true],
				      [false]))
  AM_CONDITIONAL([GRECS_COND_GIT_PARSER],
                 _GRECS_OPTION_SWITCH([parser-git],[true],
		                      [all-parsers],[true],
				      [false]))
  AM_CONDITIONAL([GRECS_COND_INSTALLHEADERS],
                 _GRECS_IF_OPTION_SET([install-headers],[true],[false]))
		   
  AC_SUBST([GRECS_SRCDIR],$1)
  AC_SUBST([GRECS_BUILD_AUX])
  AC_SUBST([GRECS_INCLUDES])
  AC_SUBST([GRECS_TESTDIR])
  AC_SUBST([GRECS_LDADD])
  AC_SUBST([GRECS_DOCDIR])
  AC_SUBST([GRECS_CHANGELOG])
  AC_SUBST([GRECS_DISTCK_AT])
  AC_SUBST([GRECS_README])
  AC_SUBST([GRECS_INCLUDES],['-I$(top_srcdir)/]grecsdir[src]')
  AC_SUBST([GRECS_HOST_PROJECT_INCLUDES])
  AC_SUBST([GRECS_DISTDOC])
  AC_SUBST([GRECS_INCLUDE_DIR],['$(pkgincludedir)'])
  
  _GRECS_OPTION_SWITCH([install],[
    LT_INIT
    GRECS_LDADD=['$(top_builddir)/]grecsdir[src/libgrecs.la']
    GRECS_DOCDIR='doc'
    GRECS_CHANGELOG=
    GRECS_DISTCK_AT=distck.at
    GRECS_README=README.standalone
    AC_CONFIG_FILES(grecsdir[src/Makefile]:grecsdir[src/Make-inst.in]
                    grecsdir[doc/Makefile])
  ],[shared],[
    LT_INIT
    GRECS_LDADD=['$(top_builddir)/]grecsdir[src/libgrecs.la']
    GRECS_CHANGELOG='#'
    GRECS_README=README.submodule
    _GRECS_IF_OPTION_SET([syntax-doc],[GRECS_DISTDOC=doc/grecs-syntax.texi])
    AC_CONFIG_FILES(grecsdir[src/Makefile]:grecsdir[src/Make-shared.in])
  ],[
    GRECS_LDADD=['$(top_builddir)/]grecsdir[src/libgrecs.a']
    GRECS_CHANGELOG='#'
    GRECS_README=README.submodule
    _GRECS_IF_OPTION_SET([syntax-doc],[GRECS_DISTDOC=doc/grecs-syntax.texi])
    AC_CONFIG_FILES(grecsdir[src/Makefile]:grecsdir[src/Make-static.in])
  ])
  AC_CONFIG_FILES(grecsdir[Makefile])
  m4_popdef([grecsdir])
])
