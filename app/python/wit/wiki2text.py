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

from wikimarkup import *
from types import TupleType
from wikins import wiki_ns_re, wiki_ns
import re
import urllib

class TextWikiMarkup (WikiMarkup):
    """
    A (general-purpose Wiki->Text translator class.
    """

    # Output width
    width = 78
    # Do not show references.
    references = False
    # Provide a minimum markup
    markup = True

    # Number of current element in the environment
    num = 0
    
    def __init__(self, *args, **keywords):
        WikiMarkup.__init__(self, *args, **keywords)
        if 'width' in keywords:
            self.width = keywords['width']
        if 'refs' in keywords:
            self.references = keywords['refs']
        if 'markup' in keywords:
            self.markup = keywords['markup']

    def xref(self, text, target):
        if text:
            return "%s (see %s) " % (text, target)
        else:
            return "see " + target

    def wiki_ns_name(self, str):
        if str in wiki_ns[self.lang]:
            return wiki_ns[self.lang][str]
        elif str in wiki_ns_re[self.lang]:
            for elt in wiki_ns_re[self.lang][str]:
                if str.beginswith(elt[0]) and str.endswith(elt[1]):
                    return elt[2]
        return None
    
    def mktgt(self, tgt, lang = None):
        if not lang:
            lang = self.lang
        return self.html_base % { 'lang' : lang } + urllib.quote(tgt)
    
    def fmtlink(self, elt, istmpl):
        arg = self.format(elt[1][0])
        if len(elt[1]) > 1:
            s = map(self.format, elt[1])
            text = s[1]
        else:
            s = None
            text = None

        if s:
            if s[0] == 'disambigR' or s[0] == 'wikiquote':
                return ""
            if len(s) > 1 and s[1] == 'thumb':
                return ""
        (qual,sep,tgt) = arg.partition(':')
        if tgt != '':
            ns = self.wiki_ns_name(qual)
            if ns:
                if ns == 'NS_IMAGE':
                    if not self.references:
                        return ""
                    text = "[%s: %s]" % (qual, text if text else arg)
                    tgt = self.image_base + '/' + \
                                 urllib.quote(tgt) + \
                                 '/250px-' + urllib.quote(tgt)
                elif ns == 'NS_MEDIA':
                    text = "[%s]" % (qual)
                else:
                    tgt = self.mktgt(tgt)
            elif not istmpl and qual in self.langtab:
                text = self.langtab[qual] + ": " + tgt
                tgt = self.mktgt(tgt, qual)
            else:
                tgt = self.mktgt(tgt)
        else:
            tgt = self.mktgt(arg)
        if self.references:
            return "%s (see %s) " % (text, tgt)
        elif not text or text == '':
            return arg
        else:
            return text

    def indent (self, lev, text):
        if text.find('\n') == -1:
            s = (" " * lev) + text 
        else:
            s = ""
            for elt in text.split('\n'):
                if elt:
                    s += (" " * lev) + elt + '\n'
            if not text.endswith('\n'):
                s = s.rstrip('\n')
#        print "IN: '%s'" % (text)
#        print "OUT: '%s'" % (s)
        return s
    
    def fmtpara(self, input):
        output = ""
        linebuf = ""
        length = 0
        for s in input.split():
            wlen = len(s)
            if linebuf.endswith("."):
                wsc = 2
            else:
                wsc = 1
            if length + wsc + wlen > self.width:
                # FIXME: fill out linebuf
                output += linebuf + '\n'
                wsc = 0
                length = 0
                linebuf = ""
            linebuf += " " * wsc + s
            length += wsc + wlen
        return output + linebuf
        
    def format(self, elt):
        if elt[0] == TEXT:
            if isinstance(elt[1],list):
                string = ""
                for s in elt[1]:
                    if string:
                        if string.endswith("."):
                            string += "  "
                        else:
                            string += " "
                    string += s
            else:
                string = elt[1]
        elif elt[0] == PARA:
            string = "";
            for x in elt[1]:
                string += self.format(x)
            string = self.fmtpara(string) + '\n\n'
        elif elt[0] == IT:
            string = ""
            for x in elt[1]:
                s = self.format(x)
                if s:
                    string += " " + s
            string = "_" + string.lstrip(" ") + "_"
        elif elt[0] == BOLD:
            string = ""
            for x in elt[1]:
                s = self.format(x)
                if s:
                    if string.endswith("."):
                        string += "  "
                    else:
                        string += " "
                string += s
            string = string.upper()
        elif elt[0] == LINK:
            string = self.fmtlink(elt, False)
        elif elt[0] == TMPL:
            s = self.fmtlink(elt, True)
            if s:
                string = '[' + s + ']'
            else:
                string = s
        elif elt[0] == BAR:
            w = self.width
            if w < 5:
                w = 5
            string = "\n" + ("-" * (w - 5)).center(w - 1) + "\n"
        elif elt[0] == HDR:
            level = elt[1]
            string = "\n" + ("*" * level) + " " + \
                      self.format(elt[2]).lstrip(" ") + "\n\n"
        elif elt[0] == REF:
            string = self.xref(self.format(elt[2]), elt[1])
        elif elt[0] == ENV:
            type = elt[1]
            lev = elt[2]
            if lev > self.width - 4:
                lev = 1
            string = ""
            n = 1
            for s in elt[3]:
                if not string.endswith("\n"):
                    string += "\n"
                x = self.format(s)
                if type == ENVUNNUM:
                    string += self.fmtpara(self.indent(lev, "- " + x.lstrip(" ")))
                elif type == ENVNUM:
                    string += self.fmtpara(self.indent(lev, "%d. %s" % (n, x)))
                    n += 1
            if not string.endswith("\n"):
                string += "\n"
        elif elt[0] == IND:
            string = (" " * elt[1]) + self.format(elt[2]) + '\n'
        elif elt[0] == SEQ:
            string = ""
            for x in elt[1]:
                if len(string) > 1 and not string[-1].isspace():
                    string += ' '
                string += self.format(x)
        else:
            string = str(elt)
        return string

    def __str__(self):
        str = ""
        for elt in self.tree:
            str += self.format(elt)
        return str

class TextWiktionaryMarkup (TextWikiMarkup):
    """
 See documentation for HtmlWiktionaryMarkup
    """
    # FIXME: It is supposed to do something about templates

