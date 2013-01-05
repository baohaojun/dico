# This file is part of GNU Dico
# Copyright (C) 1998-2000, 2008-2010, 2012 Sergey Poznyakoff
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

ACLOCAL_AMFLAGS = -I m4 -I am -I grecs/am -I gint -I imprimatur

SUBDIRS=\
 include\
 utils\
 examples\
 gint\
 gnu\
 grecs\
 lib\
 libltdl\
 dicod\
 modules\
 dico\
 scripts\
 imprimatur\
 doc\
 po
#FIXME: add these when ready: makedict client fonts

EXTRA_DIST = ChangeLog.2008

dist-hook:
	tar -C $(srcdir) -c -f - --exclude-vcs app dicoweb | \
	      tar -C $(distdir) -x -f -

#  Start Git ChangeLog from this date.
gen_start_date = 2008-05-05

.PHONY: ChangeLog
ChangeLog:
	$(AM_V_GEN)if test -d .git; then                                   \
	  git log --pretty='format:%ct  %an  <%ae>%n%n%s%n%n%b%n' |        \
	  awk -f $(top_srcdir)/@GRECS_SUBDIR@/build-aux/git2chg.awk        \
             -vsince=$(gen_start_date) > ChangeLog;                        \
        fi
