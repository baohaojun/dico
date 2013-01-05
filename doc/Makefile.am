# This file is part of GNU Dico
# Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff
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

info_TEXINFOS=dico.texi
dico_TEXINFOS=\
 fdl.texi\
 libdico.texi\
 macros.texi\
 proto.texi\
 strat.texi\
 strftime.texi

EXTRA_DIST = \
 gendocs_template

clean-local:
	rm -rf manual

AM_MAKEINFOFLAGS = @IMPRIMATUR_MAKEINFOFLAGS@
imprimatur_INPUT=$(info_TEXINFOS) $(dico_TEXINFOS)
include ../imprimatur/imprimatur.mk
CHECK_DOCS=$(top_srcdir)/@IMPRIMATUR_MODULE_DIR@/check-docs.sh

# Checking
check-options:
	@$(CHECK_DOCS) options \
	   '/OPTIONS_BEGIN/,/OPTIONS_END/s/OPTION( *\([^,][^,]*\),.*/\1/pg' \
	   '/@c option --/{s///;t a;:a;s/^\(\\option\\\|help\|version\|usage\)//;t;p}' \
	   $(top_srcdir)/dicod/cmdline.opt -- \
	   $(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) -E - \
		$(info_TEXINFOS) 

all-check-docs: imprimatur-basic-checks check-options

check-docs: 
	$(MAKE) -k all-check-docs

#

master-menu: imprimatur-master-menu
untabify: imprimatur-untabify
final: imprimatur-final

GENDOCS=gendocs.sh 

TEXI2DVI=texi2dvi -t '@set $(RENDITION)' -E

.PHONY: manual man-tar
# Make sure you set TEXINPUTS.
# TEXINPUTS=/usr/share/texmf/pdftex/plain/misc/ is ok for most distributions
manual:
	TEXINPUTS=$(srcdir):$(top_srcdir)/build-aux:$(TEXINPUTS) \
	 MAKEINFO="$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS)" \
	 TEXI2DVI="$(TEXI2DVI)" \
         TEXI2HTML="texi2html $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS)" \
	$(GENDOCS) --texi2html $(PACKAGE) '$(PACKAGE_NAME) manual'

manual.tar.bz2: manual
	tar cfj manual.tar.bz2 manual

man-tar: manual.tar.bz2
