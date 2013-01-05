# -*- coding: utf-8 -*-
#
# This file is part of GNU Dico.
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

import dico # virtual module provided by Dicod.

class DicoModule:

    def __init__ (self, *argv):
        pass

    def open (self, dbname):
        """Open the database."""
        self.dbname = dbname
        return False

    def close (self):
        """Close the database."""
        return False

    def descr (self):
        """Return a short description of the database."""
        return False

    def info (self):
        """Return a full information about the database."""
        return False

    def lang (self):
        """Optional. Return supported languages (src, dst)."""
        return ("en", "pl")

    def define_word (self, word):
        """Define a word."""
        return False

    def match_word (self, strat, word):
        """Look up a word in the database."""
        return False

    def output (self, rh, n):
        """Output Nth result from the result set."""
        return False

    def result_count (self, rh):
        """Return the number of elements in the result set."""
        return 0

    def compare_count (self, rh):
        """Return the number of comparisons performed
        when constructing the result set."""
        return 0

    def result_headers (self, rh, hdr):
        """Optional. Return a dictionary of MIME headers."""
        return hdr

    def free_result (self, rh):
        """Free any resources used by the result set."""
        pass
