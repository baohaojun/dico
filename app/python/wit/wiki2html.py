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

class HtmlWikiMarkup (WikiMarkup):
    """
    A (hopefully) general-purpose Wiki->HTML translator class.
    FIXME: 1. See WikiMarkup for a list
           2. [[official position]]s : final 's' gets after closing </a> tag.
           Should be before.
    """

    def wiki_ns_name(self, str):
        if str in wiki_ns[self.lang]:
            return wiki_ns[self.lang][str]
        elif str in wiki_ns_re[self.lang]:
            for elt in wiki_ns_re[self.lang][str]:
                if str.beginswith(elt[0]) and str.endswith(elt[1]):
                    return elt[2]
        return None            
        

    envhdr = [ "ul", "ol", "dl" ]    
    envel = [ "li", "li", "dd" ]

    def mktgt(self, tgt, lang = None):
        if not lang:
            lang = self.lang
        return self.html_base % { 'lang' : lang } + urllib.quote(tgt)

    def tmpl_term(self, s):
        if len(s) == 2:
            return s[1]
        text = None
        trans = None
        for x in s[1:]:
            m = re.match('(\w+)=', x)
            if m:
                if m.group(1) == "tr":
                    trans = x[m.end(1)+1:]
            elif not text:
                text = x
        if text:
            if trans:
                text += ' <span class="trans">[' + trans + ']</span>'
        return text

    def tmpl_proto(self, s):
        text = '<span class="proto-lang">Proto-' + s[1] + '</span>'
        if len(s) >= 4:
            n = 0
            for x in s[2:-2]:
                if n > 0:
                    text += ','
                n += 1
                text += ' <span class="proto">' + x + '</span>'
                text += ' <span class="meaning">(' + s[-2] + ')</span>'
        return text
                
    
    def fmtlink(self, elt, istmpl):
        arg = self.format(elt[1][0])
        text = None
        if len(elt[1]) > 1:
            s = map(self.format, elt[1])
            if s[0] == 'disambigR' or s[0] == 'wikiquote':
                return ""
            elif len(s) > 1 and s[1] == 'thumb':
                return ""
            text = '<span class="template">' + s[1] + '</span>'
            if istmpl:
                if re.match("t[+-]$", s[0]):
                    if len(s) > 2:
                        text = s[2]
                elif s[0] == "term":
                    text = self.tmpl_term(s)
                elif s[0] == "proto":
                    text = self.tmpl_proto(s)
                return text
            
        (qual,sep,tgt) = arg.partition(':')
        if tgt != '':
            ns = self.wiki_ns_name(qual)
            if ns:
                if ns == 'NS_IMAGE':
                    return ''
                elif ns == 'NS_MEDIA':
                    tgt = self.media_base + '/' + tgt
                else:
                    tgt = self.mktgt(tgt)
            elif not istmpl and qual in self.langtab:
                tgt = self.mktgt(tgt, qual)
                if not text or text == '':
                    text = self.langtab[qual]
            else:
                tgt = self.mktgt(tgt)
        else:
            tgt = self.mktgt(arg)
        return "<a href=\"%s\">%s</a>" % (tgt,
                                          text if (text and text != '') \
                                               else arg)
                
    def str_link(self, elt):
        return self.fmtlink(elt, False)

    def str_tmpl(self, elt):
        return self.fmtlink(elt, True)

    def str_ref(self, elt):
        target = elt[1]
        text = self.format(elt[2])
        return "<a href=\"%s\">%s</a>" % (target,
                                          text if (text and text != '') \
                                                   else target)

    def concat(self, eltlist):
        string = ""
        for x in eltlist:
            string += self.format(x)
        return string
    
    def str_it(self, elt):
        return "<i>" + self.concat(elt[1]) + "</i>"
                                          
    def str_bold(self, elt):
        return "<b>" + self.concat(elt[1]) + "</b>"
                                              
    def str_hdr(self, elt):
        level = elt[1] + 1
        if level > 4:
            level = 4
        return "<h%s>%s</h%s>" % (level, self.format(elt[2]), level)
    
    def str_bar(self):
        return "<hr/>"
    
    def str_env(self, elt):
        type = elt[1]
        lev = elt[2]
        if lev > 4:
            lev = 2
        string = ""
        for s in elt[3]:
            x = self.format(s)
            string += "<%s>%s</%s>" % (self.envel[type],
                                       self.format(s),
                                       self.envel[type])
        return "<%s>%s</%s>" % (self.envhdr[type],
                                string,
                                self.envhdr[type])

    def str_para(self, elt):
        string = "";
        for x in elt[1]:
            string += self.format(x)
        return "<p>" + string + "</p>"

    def str_ind(self, elt):
        return ("&nbsp;" * 2 * elt[1]) + self.format(elt[2])
    
    def format(self, elt):
        if elt[0] == TEXT:
            if isinstance(elt[1],list):
                string = ""
                for s in elt[1]:
                    string += s
            else:
                string = elt[1]
            return string
        elif elt[0] == PARA:
            return self.str_para(elt)
        elif elt[0] == IT:
            return self.str_it(elt)
        elif elt[0] == BOLD:
            return self.str_bold(elt)
        elif elt[0] == LINK:
            return self.str_link(elt)
        elif elt[0] == TMPL:
            return self.str_tmpl(elt)
        elif elt[0] == BAR:
            return self.str_bar()
        elif elt[0] == HDR:
            return self.str_hdr(elt)
        elif elt[0] == REF:
            return self.str_ref(elt)
        elif elt[0] == ENV:
            return self.str_env(elt)
        elif elt[0] == IND:
            return self.str_ind(elt)
        elif elt[0] == SEQ:
            string = ""
            for x in elt[1]:
                string += self.format(x)
            return string
        else:
            return str(elt)
    
    def __str__(self):
        str = ""
        for elt in self.tree:
            str += self.format(elt)
        return str

class HtmlWiktionaryMarkup (HtmlWikiMarkup):
    """
 A class for translating Wiktionary articles into HTML.
 This version does not do much, except that it tries to correctly
 format templates. But "tries" does not mean "does". The heuristics
 used here is clearly not enough to cope with it.

 1. FIXME:    
 The right solution would be to have a database of templates with their
 semantics and to decide on their rendering depending on that. E.g.
 {{term}} in en.wiktionary means "replace this with the search term".
 This, however, does not work in other wiktionaries. There are
 also more complex templates, e.g.: {{t+|bg|врата|n|p|tr=vrata|sc=Cyrl}}
 I don't know what it means. Couldn't find any documentation either.
 Again, this template does not work in other dictionaries.

 2. Capitulation notice:    
 Given the:
   1. waste amount of wiktionaries available,
   2. abundance of various templates for each wictionary,
   3. apparent lack of documentation thereof,
   4. the lack of standardized language-independent templates,
 I dont see any way to cope with the template-rendering task within a
 reasonable amount of time.
 
 Faeci quod potui, faciant meliora potentes.    
    """
