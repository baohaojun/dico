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

EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 
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

#

## -------------------------- ##
## Non-installable programs
## -------------------------- ##

INCLUDES = -I${top_srcdir}/include @GRECS_INCLUDES@
noinst_PROGRAMS = \
 levtest\
 linetrim\
 listop\
 crlftool\
 utf8

listop_SOURCES=listop.c itrsh.c itrsh.h

LDADD = ../libdico.la $(LIBINTL)

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
 crlf00.at\
 crlf01.at\
 crlf02.at\
 crlf03.at\
 crlf04.at\
 crlf05.at\
 dlev00.at\
 dlev01.at\
 lev00.at\
 lev01.at\
 list.at\
 lntrim00.at\
 lntrim01.at\
 lntrim02.at\
 lntrim03.at\
 strcasecmp.at\
 strlen.at\
 strncasecmp.at\
 testsuite.at\
 tolower.at\
 toupper.at\
 wcstrchr.at\
 wcstrcasecmp.at\
 wcstrncasecmp.at\
 wcstrncmp.at\
 wcstrcmp.at\
 wcstrstr.at

TESTSUITE = $(srcdir)/testsuite
M4=m4

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AM_V_GEN)$(AUTOTEST) -I $(srcdir) testsuite.at -o $@.tmp
	$(AM_V_at)mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	@test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE) 
	@$(SHELL) $(TESTSUITE)

# Run the test suite on the *installed* tree.
#installcheck-local:
#	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=$(exec_prefix)/bin


