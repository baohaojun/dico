#  This file is part of GNU Dico.
#  Copyright (C) 2008-2009, 2012 Wojciech Polak
#
#  GNU Dico is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
#
#  GNU Dico is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

from django.utils.translation import ugettext_noop

DUMMY_TRANSLATIONS = (
    # gnu.org.ua
    ugettext_noop('Match words exactly'),
    ugettext_noop('Match word prefixes'),
    ugettext_noop('Match using SOUNDEX algorithm'),
    ugettext_noop('Match everything (experimental)'),
    ugettext_noop('Match headwords within given Levenshtein distance'),
    ugettext_noop('Match headwords within given Levenshtein distance (normalized)'),
    ugettext_noop('Match headwords within given Damerau-Levenshtein distance'),
    ugettext_noop('Match headwords within given Damerau-Levenshtein distance (normalized)'),
    ugettext_noop('POSIX 1003.2 (modern) regular expressions'),
    ugettext_noop('Old (basic) regular expressions'),
    ugettext_noop('Match word suffixes'),
    ugettext_noop('Reverse search in Quechua databases'),
    # dict.org
    ugettext_noop('Match headwords exactly'),
    ugettext_noop('Match prefixes'),
    ugettext_noop('Match substring occurring anywhere in a headword'),
    ugettext_noop('Match suffixes'),
    ugettext_noop('Match headwords within Levenshtein distance one'),
    ugettext_noop('Match separate words within headwords'),
)
