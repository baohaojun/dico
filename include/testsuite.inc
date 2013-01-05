# This file is part of GNU Dico -*- Autotest -*-
# Copyright (C) 2012 Sergey Poznyakoff
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

# Require a reasonably recent autotest.
m4_version_prereq([2.52g])

m4_define([AT_SKIP_TEST],[exit 77])

dnl ------------------------------------------------------------
dnl DICO_VERSION(TOOL)  -- checks the version output from TOOL
dnl
m4_define([DICO_VERSION],[
AT_SETUP([$1 version])
AT_KEYWORDS([version])
AT_CHECK([$1 --version | sed '1{s/ *[\[.*\]]//;q;}' ],
         [0],
         [$1 (AT_PACKAGE_NAME) AT_PACKAGE_VERSION
],
[],
[cat >.xfailfile <<'_EOT'

==============================================================
WARNING: Not using the proper version, *all* checks dubious...
==============================================================
_EOT
],
[rm -f $[]XFAILFILE])
AT_CLEANUP
])

dnl ------------------------------------------------------------
dnl DICOD_RUN([config])
dnl
m4_define([DICOD_RUN],[dnl
dicod --config m4_if($1,,.,$1)/dicod.conf --stderr -i < input | dnl
    tr -d '\r' | sed 's/^\(2[[25][0-9]]\) .*/\1/;s/ *$//'])
    

m4_define([dico_quote], [m4_if([$#], [0], [], [[$*]])])

dnl AT_DICOD([TEXT],[OUTPUT],[ERROR])
m4_define([AT_DICOD],[
AT_DATA([input],dico_quote($1))
AT_CHECK([DICOD_RUN($abs_builddir)],
[0],
[$2],
[$3])])

