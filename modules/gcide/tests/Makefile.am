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

EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 dicod.cfin
DISTCLEANFILES       = atconfig $(check_SCRIPTS) GCIDE.IDX
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)

SUFFIXES = .cfin .conf

.cfin.conf:
	$(AM_V_GEN)sed 's|~moddir~|$(abs_top_builddir)/modules|g;s|~dictdir~|$(abs_srcdir)/dict|g;s|~idxdir~|$(abs_builddir)|g' $< > $@

noinst_DATA = dicod.conf
CLEANFILES = dicod.conf

dist-hook:
	tar -C $(srcdir) -c -f - dict/CIDE.[A-Z] dict/INFO | \
	      tar -C $(distdir) -x -f -

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


## ------------ ##
## Auxiliaries. ##
## ------------ ##

noinst_PROGRAMS=degcide greek exgrk
degcide_LDADD = ../libgcide.la ../../../lib/libdico.la $(LIBICONV) @LIBINTL@
greek_LDADD = ../libgcide.la ../../../lib/libdico.la $(LIBICONV) @LIBINTL@
INCLUDES = \
 -I$(top_srcdir)/include\
 -I$(top_builddir)/include\
 -I$(top_srcdir)/modules/gcide\
 @GRECS_INCLUDES@

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
 all.at\
 autoidx.at\
 def00.at\
 def01.at\
 def02.at\
 def03.at\
 def04.at\
 def05.at\
 descr.at\
 exact.at\
 greek.at\
 idx.at\
 info.at\
 markup.at\
 nopr.at\
 prefix.at\
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

