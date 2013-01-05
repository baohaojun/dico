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
bin_PROGRAMS=dicod
dicod_SOURCES=\
 accesslog.c\
 acl.c\
 alias.c\
 auth.c\
 capa.c\
 ckpass.c\
 cmdline.c\
 commands.c\
 dbtext.c\
 dicod.c\
 gsasl.c\
 ident.c\
 lang.c\
 lev.c\
 lint.c\
 loader.c\
 main.c\
 markup.c\
 mime.c\
 ostream.c\
 regex.c\
 server.c\
 stat.c\
 stratcl.c\
 xidle.c\
 xversion.c

noinst_HEADERS=dicod.h

incdir=$(pkgdatadir)/$(VERSION)/include
AM_CFLAGS = \
 -DSYSCONFIG=\"$(sysconfdir)\"\
 -DLOCALSTATEDIR=\"$(localstatedir)\"\
 -DDICO_MODDIR=\"$(DICO_MODDIR)\"\
 -DDEFAULT_VERSION_INCLUDE_DIR=\"$(incdir)\"\
 -DDEFAULT_INCLUDE_DIR=\"$(pkgdatadir)/include\"\
 -DDEFAULT_PREPROCESSOR="$(DEFAULT_PREPROCESSOR)"

AM_YFLAGS = -dtv
AM_LFLAGS = -dvp

LDADD=\
 ../lib/libxdico.a\
 ../gnu/libgnu.a\
 ../lib/libextra.a\
 ../lib/libdico.la\
 @LIBINTL@ @LIBICONV@\
 @LIBDICOSASL@\
 @GSASL_LIBS@\
 @LIBLTDL@  

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/gnu -I../gnu @GRECS_INCLUDES@ @LTDLINCL@
# dicod_DEPENDENCIES=@LTDLDEPS@

SUFFIXES=.opt .c

.opt.c:
	$(AM_V_GEN)m4 -s $(top_srcdir)/@GRECS_SUBDIR@/build-aux/getopt.m4 $< > $@

BUILT_SOURCES=cmdline.c 
EXTRA_DIST=cmdline.opt
