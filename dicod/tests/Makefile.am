# This file is part of GNU Dico
# Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff
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

EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 passwd group
DISTCLEANFILES       = atconfig $(check_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)

## ------------ ##
## package.m4.  ##
## ------------ ##

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	$(AM_V_GEN){                                      \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

## --------------- ##
## The echo module ##
## --------------- ##

## This module is hardly usable outside of the Dico test suite, but
## it must be an installable target nevertheless, otherwise libtool
## will not create a shared library and `make check' will fail.

moddir=@DICO_MODDIR@

mod_LTLIBRARIES=echo.la

echo_la_SOURCES = echo.c
echo_la_LIBADD = ../../lib/libdico.la
echo_la_LDFLAGS = -module -avoid-version -no-undefined

## ------------ ##
## Auxiliaries. ##
## ------------ ##

noinst_PROGRAMS=apopauth
apopauth_SOURCES=apopauth.c
apopauth_LDADD=\
 ../../lib/libxdico.a\
 ../../gnu/libgnu.a\
 ../../lib/libextra.a\
 ../../lib/libdico.la\
 $(READLINE_LIBS)\
 @LIBINTL@ @LIBICONV@\
 @LIBDICOSASL@\
 @GSASL_LIBS@\
 @LIBLTDL@

INCLUDES = \
 -I$(top_srcdir)/include\
 -I$(top_builddir)/include\
 -I$(top_srcdir)/gnu\
 -I$(top_builddir)/gnu\
 @GRECS_INCLUDES@

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
 alias.at\
 apop.at\
 def.at\
 descr.at\
 info.at\
 help00.at\
 help01.at\
 help02.at\
 help03.at\
 match.at\
 nodef.at\
 nomatch.at\
 showdb.at\
 showstrat.at\
 startup.at\
 vis00.at\
 vis01.at\
 vis02.at\
 testsuite.at

TESTSUITE = $(srcdir)/testsuite
M4=m4

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AM_V_GEN)$(AUTOTEST) -I $(srcdir) -I $(top_srcdir)/include testsuite.at -o $@.tmp
	$(AM_V_at)mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	@test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE) 
	@$(SHELL) $(TESTSUITE)

