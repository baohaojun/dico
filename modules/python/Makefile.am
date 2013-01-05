# This file is part of GNU Dico
# Copyright (C) 2008, 2012 Wojciech Polak
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

SUBDIRS = . tests

moddir=@DICO_MODDIR@

mod_LTLIBRARIES=python.la

python_la_SOURCES = python.c
python_la_LIBADD = ../../lib/libdico.la @PYTHON_LIBS@
AM_LDFLAGS = -module -avoid-version -no-undefined -rpath $(moddir)
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include @PYTHON_INCLUDES@
