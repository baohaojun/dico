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

SUBDIRS=. tests

moddir=@DICO_MODDIR@

mod_LTLIBRARIES=guile.la

guile_la_SOURCES = guile.c
guile_la_LIBADD = ../../lib/libdico.la @GUILE_LIBS@
AM_LDFLAGS = -module -avoid-version -no-undefined -rpath $(moddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include @GRECS_INCLUDES@ @GUILE_INCLUDES@

SUFFIXES =
BUILT_SOURCES =
EXTRA_DIST = guile.x
DOT_X_FILES = guile.x
CLEANFILES=
DISTCLEANFILES=
sitedir = @GUILE_SITE@/$(PACKAGE)
site_DATA=

include ../../gint/gint.mk
