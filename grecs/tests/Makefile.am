# This file is part of grecs - Gray's Extensible Configuration System
# Copyright (C) 2007, 2009-2012 Sergey Poznyakoff
#
# Grecs is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# Grecs is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Grecs.  If not, see <http://www.gnu.org/licenses/>.

EXTRA_DIST = \
 $(TESTSUITE_AT)\
 testsuite\
 package.m4\
 gcf1.conf\
 bind.conf\
 bind.keys\
 bind.int.conf\
 bind.ext.conf\
 bind.dlz\
 meta1.conf\
 git.conf

DISTCLEANFILES       = atconfig $(check_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)

PARSER_DEFS =

if GRECS_COND_META1_PARSER
  PARSER_DEFS += ENABLE_META1_PARSER
endif

if GRECS_COND_BIND_PARSER
  PARSER_DEFS += ENABLE_BIND_PARSER
endif

if GRECS_COND_GIT_PARSER
  PARSER_DEFS += ENABLE_GIT_PARSER 
endif

## ------------ ##
## package.m4.  ##
## ------------ ##

$(srcdir)/package.m4: $(top_srcdir)/configure.ac $(top_srcdir)/@GRECS_SUBDIR@/am/grecs.m4
	$(AM_V_GEN){                                      \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
          echo 'm4_define([GRECS_DISTCK_AT],      [@GRECS_DISTCK_AT@])'; \
          for var in $(PARSER_DEFS); do \
            echo "m4_define([$$var],1)"; \
          done; \
	} >$(srcdir)/package.m4

#

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
 bind00.at\
 cfhelp.at\
 empty.at\
 format00.at\
 format01.at\
 format02.at\
 glob00.at\
 glob01.at\
 glob02.at\
 glob03.at\
 grecs00.at\
 enum.at\
 join.at\
 locus00.at\
 locus01.at\
 locus02.at\
 locus-bind.at\
 locus-git.at\
 locus-meta1.at\
 parser-bind.at\
 parser-git.at\
 parser-meta1.at\
 path-locus.at\
 peek00.at\
 peek01.at\
 peek02.at\
 peek03.at\
 reduce00.at\
 reduce01.at\
 reduce02.at\
 reduce03.at\
 set.at\
 testsuite.at\
 @GRECS_DISTCK_AT@

TESTSUITE = $(srcdir)/testsuite
M4=m4

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AUTOTEST) -I $(srcdir) testsuite.at -o $@.tmp
	mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE) $(noinst_PROGRAMS)
	$(SHELL) $(TESTSUITE)

# Run the test suite on the *installed* tree.
#installcheck-local:
#	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=$(exec_prefix)/bin

noinst_PROGRAMS = \
 distck\
 gcffmt\
 gcfenum\
 gcfpeek\
 gcfset\
 gcfver

LDADD = @GRECS_LDADD@ $(LIBINTL)
INCLUDES = @GRECS_INCLUDES@ @GRECS_HOST_PROJECT_INCLUDES@



