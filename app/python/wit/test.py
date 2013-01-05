#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2008 Sergey Poznyakoff
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import unittest
import wiki2html

class TestMarkupParserBasic (unittest.TestCase):

    def test_colon(self):
        self.assert_(self.__test('colon'))
        pass

    def test_headings(self):
        self.assert_(self.__test('headings'))
        pass

    def test_hz(self):
        self.assert_(self.__test('hz'))
        pass

    def test_numlist(self):
        self.assert_(self.__test('numlist'))
        pass

    def test_unlist(self):
        self.assert_(self.__test('unlist'))
        pass
    
    def test_door(self):
        self.assert_(self.__test('door'))
        pass

    def test_drzwi(self):
        self.assert_(self.__test('drzwi'))
        pass

    def __test(self, filename):
        name_in  = 'testdata/' + filename + '.wiki'
        name_out = 'testdata/' + filename + '.html'
        fh = open(name_out)
        buf = ''.join(fh.readlines()).strip()
        hwm = wiki2html.HtmlWiktionaryMarkup(filename=name_in, lang="pl")
        hwm.parse()

        if str(hwm).strip() == buf:
            return True

        # fail
        print "\n>>>%s<<<" % buf
        print ">>>%s<<<" % str(hwm).strip()
        return False

if __name__ == '__main__':
    unittest.main()

