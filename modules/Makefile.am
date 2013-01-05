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

if GUILE_COND
  GUILE_DIR=guile
endif
if PYTHON_COND
  PYTHON_DIR=python
endif
if LDAP_COND
  LDAP_DIR=ldap
endif
if PCRE_COND
  PCRE_DIR=pcre
endif
if WORDNET_COND
  WORDNET_DIR=wordnet
endif
if PAM_COND
  PAM_DIR=pam
endif

SUBDIRS = \
 outline\
 dict.org\
 gcide\
 $(GUILE_DIR)\
 $(PYTHON_DIR)\
 stratall\
 substr\
 $(LDAP_DIR)\
 $(PAM_DIR)\
 $(PCRE_DIR)\
 $(WORDNET_DIR)\
 word\
 nprefix
EXTRA_DIST = template.c template.py template.scm
