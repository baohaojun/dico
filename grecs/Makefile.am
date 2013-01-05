# This file is part of Grecs
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

ACLOCAL_AMFLAGS = -I am
SUBDIRS=. src @GRECS_TESTDIR@ @GRECS_DOCDIR@
EXTRA_DIST=build-aux/yyrename @GRECS_BUILD_AUX@ @GRECS_DISTDOC@ gitid.h

noinst_HEADERS = gitid.h
BUILT_SOURCES = gitid.h README

.PHONY: gitid.h
gitid.h:
	@if test -d .git; then \
           url=`git config --get remote.origin.url | sed 's|.*://||;s|/gitroot/|/|'`; \
           if test "$$url" = "git.gnu.org.ua/grecs.git"; then \
	      dirty=`git diff-index --name-only HEAD 2>/dev/null` || dirty=;\
              test -n "$$dirty" && dirty="-dirty"; \
	      ID=`git log -1 --pretty='format:%H-%ct-%ae'`$$dirty;\
	      echo "#define GRECS_GIT_ID \"$$ID\"" > gitid.tmp; \
	      cmp gitid.tmp gitid.h >/dev/null 2>&1 || mv gitid.tmp gitid.h; \
	      rm -f gitid.tmp; \
           fi; \
	fi; 

README:
	test -f @GRECS_README@ && cp @GRECS_README@ README

@GRECS_CHANGELOG@.PHONY: ChangeLog
@GRECS_CHANGELOG@ChangeLog:
@GRECS_CHANGELOG@	$(AM_V_GEN)if test -d .git; then                                   \
@GRECS_CHANGELOG@          git log --pretty='format:%ct  %an  <%ae>%n%n%s%n%n%b%n' |        \
@GRECS_CHANGELOG@            awk -f $(top_srcdir)/build-aux/git2chg.awk > ChangeLog;        \
@GRECS_CHANGELOG@        fi
