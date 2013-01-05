#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2008, 2009 Sergey Poznyakoff
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

import sys
import re
from types import *

__all__ = [ "BaseWikiMarkup", "WikiMarkup",
            "NIL", "TEXT", "DELIM", "NL", "PARA",
            "IT", "BOLD", "LINK", "TMPL",
            "BAR", "HDR", "REF", "ENV", "IND", "SEQ",
            "ENVUNNUM", "ENVNUM", "envtypes" ]

delim = re.compile("^==+|==+[ \\t]*$|(^----$)|^\\*+|^#+|^:+|(\\[\\[)|\\[|(\\{\\{)|(\\]\\])|\\]|(\\}\\})|\\||(\\'\\'\\'?)")

NIL = 0
TEXT = 1
DELIM = 2
NL = 3

PARA = 4
IT = 5
BOLD = 6
LINK = 7
TMPL = 8
BAR = 9
HDR = 10
REF = 11
ENV = 12
IND = 13
SEQ = 14

# Environment types:
# Unnumbered list
ENVUNNUM = 0
# Numbered list
ENVNUM   = 1
envtypes = [ "*", "#" ]

class BaseWikiMarkup:

    toklist = None
    tokind = 0
    tree = None

    debug_level = 0
    
    def dprint(self, lev, fmt, *argv):
        if self.debug_level >= lev:
            print "[DEBUG]", fmt % argv
    
    def tokread(self):
        line = None
        pos = 0
        while 1:
            if (not line or pos == len(line)):
                try:
                    line = self.input()
                    pos = 0
                except StopIteration:
                    line = u''
                    
            if not line or line == "":
                self.dprint(100, "YIELD: NIL")
                yield(NIL,)
                break

            if line == '\n':
                self.dprint(100, "YIELD: NL")
                yield(NL,line)
                line = None
                continue

            self.dprint(100, "LINE: %s", line[pos:])
            m = delim.search(line, pos)
            
            if m:
                if (pos < m.start(0)):
                    self.dprint(100, "YIELD: TEXT %s", line[pos:m.start(0)])
                    yield(TEXT, line[pos:m.start(0)])
                pos = m.end(0)
                if m.group(0)[0] in envtypes and line[pos] == ":":
                    self.dprint(100, "YIELD: DELIM %s, True", m.group(0))
                    yield(DELIM, m.group(0), True)
                    pos += 1
                else:
                    self.dprint(100, "YIELD: DELIM %s", m.group(0))
                    yield(DELIM, m.group(0))
            else:
                if line[-1] == '\n':
                    self.dprint(100, "YIELD: TEXT %s", line[pos:-1])
                    if line[pos:-1] != '':
                        yield(TEXT, line[pos:-1])
                    self.dprint(100, "YIELD: NL")
                    yield(NL,'\n')
                else:
                    self.dprint(100, "YIELD: TEXT %s", line[pos:])
                    yield(TEXT, line[pos:])
                line = None

    def input(self):
        return None

    def tokenize(self):
        self.toklist = []
        for tok in self.tokread():
            self.toklist.append(tok)
        # Determine and fix up the ordering of bold and italic markers
        # This helps correctly parse inputs like:
        #   '''''Door''' files kan ik niet op tijd komen.''
        stack = []
        for i in range(0,len(self.toklist)):
            if self.toklist[i][0] == DELIM \
                    and (self.toklist[i][1] == "''" \
                             or self.toklist[i][1] == "'''"):
                if len(stack) > 0 \
                        and self.toklist[stack[-1]][1] == self.toklist[i][1]:
                    stack.pop()
                elif len(stack) > 1:
                    x = self.toklist[stack[-2]]
                    self.toklist[stack[-2]] = self.toklist[stack[-1]]
                    self.toklist[stack[-1]] = x
                    stack.pop()
                else:
                    stack.append(i)

    def peektkn(self):
        return self.toklist[self.tokind]

    def setkn(self,val):
        self.toklist[self.tokind] = val
    
    def getkn(self):
        tok = self.toklist[self.tokind]
        if tok[0] != NIL:
            self.tokind = self.tokind + 1
        return tok

    def ungetkn(self):
        self.tokind = self.tokind - 1
        return self.toklist[self.tokind]

    def parse_fontmod(self,delim,what):
        self.dprint(80, "ENTER parse_fontmod(%s,%s), tok %s",
                    delim, what, self.peektkn())
        seq = []
        textlist = []
        while 1:
            tok = self.getkn()
            if tok[0] == TEXT:
                textlist.append(tok[1])
            elif tok[0] == DELIM:
                if tok[1] == delim:
                    break
                elif self.is_inline_delim(tok):
                    if textlist:
                        seq.append((TEXT, textlist))
                        textlist = []
                    x = self.parse_inline(tok)
                    if x:
                        seq.append(x)
                    else:
                        self.dprint(80, "LEAVE parse_fontmod=%s", "None")
                        return None
                else:
                    self.dprint(80, "LEAVE parse_fontmod=None")
                    return None
            elif tok[0] == NL:
                if self.peektkn()[0] == NL:
                    self.dprint(80, "LEAVE parse_fontmod=None")
                    return None
                seq.append((TEXT, '\n'))
            else:
                self.dprint(80, "LEAVE parse_fontmod=None")
                return None
        if textlist:
            seq.append((TEXT, textlist))
        res = (what, seq)
        self.dprint(80, "LEAVE parse_fontmod=%s", res)    
        return res

    def parse_link(self, type, delim):
        self.dprint(80, "ENTER parse_link(%s,%s), tok %s",
                    type, delim, self.peektkn())
        subtree = []
        list = []
        while 1:
            tok = self.getkn()
            if tok[0] == DELIM:
                if tok[1] == delim:
                    if list:
                        subtree.append((SEQ,list))
                    break
                elif tok[1] == "|":
                    if len(list) > 1:
                        subtree.append((SEQ,list))
                    elif list:
                        subtree.append(list[0])
                    list = []
                else:
                    x = self.parse_inline(tok)
                    if x:
                        list.append(x)
                    else:
                        self.dprint(80, "LEAVE parse_link=%s", "None")
                        return None
            elif tok[0] == TEXT:
                list.append(tok)
            else:
                self.dprint(80, "LEAVE parse_link=%s", "None")
                return None
        self.dprint(80, "LEAVE parse_link=(%s,%s)", type, subtree)
        return (type, subtree)

    def parse_ref(self):
        self.dprint(80, "ENTER parse_ref, tok %s", self.peektkn())
        list = []
        while 1:
            tok = self.getkn()
            if tok[0] == DELIM:
                if tok[1] == "]":
                    break
                else:
                    x = self.parse_inline(tok)
                    if x:
                        list.append(x)
                    else:
                        self.dprint(80, "LEAVE parse_ref=%s", "None")
                        return None
            elif tok[0] == TEXT:
                list.append(tok)
            elif tok[0] == NL:
                list.append((TEXT, '\n'))
                continue
            else:
                self.dprint(80, "LEAVE parse_ref=%s", "None")
                return None
        if len(list) == 0 or list[0][0] != TEXT:
            self.dprint(80, "LEAVE parse_ref=%s", "None")
            return None
        (ref,sep,text) = list[0][1].partition(' ')
        ret = (REF, ref, (SEQ, [(TEXT, text)] + list[1:]))
        self.dprint(80, "LEAVE parse_ref= %s", ret)
        return ret

    inline_delims = [ "''", "'''", "[", "[[", "{{" ]

    def is_inline_delim(self, tok):
        return tok[0] == DELIM and tok[1] in self.inline_delims
    def is_block_delim(self, tok):
        return tok[0] == DELIM and tok[1] not in self.inline_delims
    
    def parse_inline(self, tok):
        self.dprint(80, "ENTER parse_inline(%s), tok %s", tok, self.peektkn())
        tokind = self.tokind
        if tok[1] == "''":
            x = self.parse_fontmod(tok[1], IT)
        elif tok[1] == "'''":
            x = self.parse_fontmod(tok[1], BOLD)
        elif tok[1] == "[":
            x = self.parse_ref()
        elif tok[1] == "[[":
            x = self.parse_link(LINK, "]]")
        elif tok[1] == "{{":
            x = self.parse_link(TMPL, "}}")
        else: # FIXME
            self.dprint(80, "LEAVE parse_inline=%s", "None")
            x = None
        if not x:
            self.tokind = tokind
        self.dprint(80, "LEAVE parse_inline=%s", x)
        return x

    def parse_para(self):
        self.dprint(80, "ENTER parse_para, tok %s", self.peektkn())
        seq = []
        textlist = []
        while 1:
            tok = self.getkn()
            if tok[0] == TEXT:
                textlist.append(tok[1])
            elif tok[0] == NL:
                tok = self.getkn()
                if tok[0] == NL or tok[0] == NIL:
                    break
                else:
                    self.ungetkn()
                    if self.is_block_delim(tok):
                        break
                textlist.append('\n')
            elif tok[0] == NIL:
                break
            elif tok[0] == DELIM:
                if self.is_inline_delim(tok):
                    if textlist:
                        seq.append((TEXT, textlist))
                        textlist = []
                    x = self.parse_inline(tok)
                    if x:
                        seq.append(x)
                    else:
                        seq.append(tok)
                        break
                else:
                    seq.append((TEXT,tok[1]))
                #    self.ungetkn()
                    break
        if textlist:
            seq.append((TEXT, textlist))
        self.dprint(80, "LEAVE parse_para=%s", seq)
        return (PARA, seq)

    def parse_header(self, delim):
        self.dprint(80, "ENTER parse_header(%s), tok %s", delim, self.peektkn())
        list = []
        while 1:
            tok = self.getkn()
            if tok[0] == NIL:
                self.dprint(80, "LEAVE parse_header=%s", "None")        
                return None
            elif tok[0] == TEXT:
                list.append(tok)
            elif tok[0] == DELIM:
                if tok[1] == delim:
                    if self.peektkn()[0] == NL:
                        break
                    else:
                        self.dprint(80, "LEAVE parse_header=%s", "None")
                        return None
                else:
                    x = self.parse_inline(tok)
                    if x:
                        list.append(x)
                    else:
                        self.dprint(80, "LEAVE parse_header=%s", "None")
                        return None #FIXME?
            else:
                self.dprint(80, "LEAVE parse_header=%s", "None")
                return None
        self.dprint(80, "LEAVE parse_header=(HDR, %s, (SEQ,%s))",len(delim)-1,list)
        return (HDR,len(delim)-1,(SEQ,list))


    def parse_line(self):
        self.dprint(80, "ENTER parse_line, tok %s", self.peektkn())
        list = []
        while 1:
            tok = self.getkn()
            if tok[0] == NL or tok[0] == NIL:
                break
            elif tok[0] == TEXT:
                list.append(tok)
            elif tok[0] == DELIM and tok[1][0] == ":":
                list.append(self.parse_indent(len(tok[1])))
                break
            else:
                x = self.parse_inline(tok)
                if x:
                    list.append(x)
                else:
                    list.append(tok)
        self.dprint(80, "LEAVE parse_line=(SEQ, %s)", list)
        return (SEQ, list)
    
    def parse_env(self, type, lev):
        self.dprint(80, "ENTER parse_env(%s,%s), tok %s",type,lev,self.peektkn())
        list = []
        while 1:
            tok = self.getkn()
            if tok[0] == DELIM and tok[1][0] in envtypes and type == envtypes.index(tok[1][0]):
                if len(tok[1]) < lev:
                    self.ungetkn()
                    break
                elif len(tok[1]) > lev:
                    self.ungetkn()
                    elt = self.parse_env(type, len(tok[1]))
                else:
                    elt = self.parse_line()
                    if len(tok) == 2:
                        list.append(elt)
                        continue
                    
                if list[-1][0] != SEQ:
                    x = list[-1]
                    list[-1] = (SEQ, [x])
                list[-1][1].append(elt)
            else:
                self.ungetkn()
                break
        self.dprint(80, "LEAVE parse_env=(ENV, %s, %s, %s)", type, lev, list)
        return (ENV, type, lev, list)

    def parse_indent(self, lev):
        self.dprint(80, "ENTER parse_indent(%s), tok %s", lev, self.peektkn())
        x = (IND, lev, self.parse_line())
        self.dprint(80, "LEAVE parse_indent=%s", x)
        return x
    
    def parse0(self):
        tok = self.getkn()
        toktype = tok[0]
        if toktype == NIL:
            return None
        elif toktype == TEXT:
            self.ungetkn()
            return self.parse_para()
        elif toktype == DELIM:
            if tok[1] == "----":
                return (BAR,)
            elif tok[1][0:2] == "==":
                return self.parse_header(tok[1])
            elif tok[1][0] in envtypes:
                type = envtypes.index(tok[1][0])
                lev = len(tok[1])
                self.ungetkn()
                return self.parse_env(type, lev)
            elif tok[1][0] == ":":
                return self.parse_indent(len(tok[1]))
            else:
                self.ungetkn()
                return self.parse_para()
        elif toktype == NL:
            return (TEXT, '\n')
#            return self.parse0()
    
    def parse(self):
        if not self.toklist:
            self.tokenize()
        self.dprint(90, "TOKLIST: %s", self.toklist)
        self.tokind = 0
        self.tree = []
        while 1:
            subtree = self.parse0()
            if subtree == None:
                break
            self.tree.append(subtree)
        self.dprint(70, "TREE: %s", self.tree)

    def __str__(self):
        return str(self.tree)


class WikiMarkup (BaseWikiMarkup):
    """
    A derived class, that supplies a basic input method.
    
    Three types of inputs are available:

    1. filename=<file>
    The file <file> is opened and used for input.
    2. file=<file>
    The already opened file <file> is used for input.
    3. text=<string>
    Input is taken from <string>, line by line.

    Usage:

    obj = WikiMarkup(arg=val)
    obj.parse
    ... Do whatever you need with obj.tree ...

    """
    file = None
    text = None
    lang = 'en'
    html_base = 'http://%(lang)s.wiktionary.org/wiki/'
    image_base = 'http://upload.wikimedia.org/wikipedia/commons/thumb/a/bf'
    media_base = 'http://www.mediawiki.org/xml/export-0.3'
    
    def __init__(self, *args, **keywords):
        for kw in keywords:
            if kw == 'file':
                self.file = keywords[kw]
            elif kw == 'filename':
                self.file = open(keywords[kw])
            elif kw == 'text':
                self.text = keywords[kw].split("\n")
            elif kw == 'lang':
                self.lang = keywords[kw]
            elif kw == 'html_base':
                self.html_base = keywords[kw]
            elif kw == 'image_base':
                self.image_base = keywords[kw]
            elif kw == 'media_base':
                self.media_base = keywords[kw]

    def __del__(self):
        if self.file:
            self.file.close()

    def input(self):
        if self.file:
            return self.file.readline()
        elif self.text:
            return self.text.pop(0) + '\n'
        else:
            return None

    def is_lang_link(self, elt):
        if elt[0] == LINK and isinstance(elt[1],list) and len(elt[1]) == 1:
            if elt[1][0][0] == TEXT:
                m = re.match('([\w-]+):', elt[1][0][1])
                if m: # and m.group(1) in self.langtab:
                    return True
            elif elt[1][0][0] == SEQ and len(elt[1][0][1]) == 1 and\
                    elt[1][0][1][0][0] == TEXT:
                m = re.match('([\w-]+):',elt[1][0][1][0][1])
                if m: # and m.group(1) in self.langtab:
                    return True
        return False
    
    def is_empty_text(self, elt):
        if elt[0] == TEXT:
            if isinstance(elt[1],list):
                for s in elt[1]:
                    if re.search('\w', s):
                        return False
            elif re.search('\w', elt[1]):
                return False
            return True
        return False

    def is_empty_para(self, seq):
        for x in seq:             
            if not (self.is_lang_link(x) or self.is_empty_text(x)):
                return False
        return True
    
    def parse(self):
        BaseWikiMarkup.parse(self)
        # Remove everything before the first header
        for i in range(0, len(self.tree)):
            if self.tree[i][0] == HDR:
                self.tree = self.tree[i:]
                break
        # Remove trailing links
        for i in range(len(self.tree)-1, 0, -1):
            if self.tree[i][0] == PARA \
                    and not self.is_empty_para(self.tree[i][1]):
                self.tree = self.tree[0:i+1]
                break
                    
        
    # ISO 639 
    langtab = {
        "aa": "Afar",            # Afar
	"ab": "Аҧсуа",           # Abkhazian
	"ae": None,              # Avestan
	"af": "Afrikaans",       # Afrikaans
	"ak": "Akana",           # Akan
        "als": "Alemannisch",
	"am": "አማርኛ",            # Amharic
	"an": "Aragonés",        # Aragonese
        "ang": "Englisc",
	"ar": "العربية" ,        # Arabic
        "arc": "ܐܪܡܝܐ",
	"as": "অসমীয়া",         # Assamese
        "ast": "Asturian", 
	"av": "Авар",            # Avaric
	"ay": "Aymar",           # Aymara
	"az": "Azərbaycan" ,     # Azerbaijani

	"ba": "Башҡорт",         # Bashkir
        "bar": "Boarisch", 	
        "bat-smg": "Žemaitėška",
        "bcl": "Bikol",
	"be": "Беларуская",      # Byelorussian; Belarusian
        "be-x-old": "Беларуская (тарашкевіца)",
	"bg": "Български",       # Bulgarian
	"bh": "भोजपुरी",  # Bihari
	"bi": "Bislama",         # Bislama
	"bm": "Bamanankan",      # Bambara
	"bn": "বাংলা" ,          # Bengali; Bangla
	"bo": "བོད་སྐད",         # Tibetan
        "bpy": "ইমার ঠার/বিষ্ণুপ্রিয়া মণিপুরী" ,
	"br": "Brezhoneg" ,      # Breton
	"bs": "Bosanski" ,       # Bosnian
        "bug": "Basa Ugi",
        "bxr": "Буряад",

	"ca": "Català" ,         # Catalan
        "cbk-zam": "Chavacano de Zamboanga",
        "cdo": "Mìng-dĕ̤ng-ngṳ̄",
        "cho": "Choctaw",
	"ce": "Нохчийн",         # Chechen
        "ceb": "Sinugboanong Binisaya" , # Cebuano
	"ch": "Chamor",          # Chamorro
        "chr": "ᏣᎳᎩ",
        "chy": "Tsetsêhestâhese",
	"co": "Cors",            # Corsican
	"cr": "Nehiyaw",         # Cree
        "crh": "Qırımtatarca",
	"cs": "Česky" ,          # Czech
        "csb": "Kaszëbsczi",
	"c": "Словѣньскъ",       # Church Slavic
	"cv": "Чăваш",           # Chuvash
	"cy": "Cymraeg" ,        # Welsh

	"da": "Dansk" ,          # Danish
	"de": "Deutsch" ,        # German
        "diq": "Zazaki",         # Dimli (Southern Zazaki)
        "dsb": "Dolnoserbski",
	"dv": "ދިވެހިބަސް",      # Divehi
	"dz": "ཇོང་ཁ",           # Dzongkha; Bhutani

	"ee": "Eʋegbe",          # Ewe
	"el": "Ελληνικά" ,       # Greek
        "eml": "Emiliàn e rumagnòl",
	"en": "English" ,        # English
        "eo": "Esperanto" ,
	"es": "Español" ,        # Spanish
	"et": "Eesti" ,          # Estonian
	"eu": "Euskara" ,         # Basque
        "ext": "Estremeñ",

	"fa": "فارسی" ,          # Persian
	"ff": "Fulfulde",        # Fulah
	"fi": "Suomi" ,          # Finnish
        "fiu-vro": "Võro",
	"fj": "Na Vosa Vakaviti",# Fijian; Fiji
	"fo": "Føroyskt" ,       # Faroese
	"fr": "Français" ,       # French
        "frp": "Arpitan",
        "fur": "Furlan",
	"fy": "Frysk",           # Frisian

	"ga": "Gaeilge",         # Irish
        "gan": "贛語 (Gànyŭ)",
	"gd": "Gàidhlig",        # Scots; Gaelic
	"gl": "Gallego" ,        # Gallegan; Galician
        "glk": "گیلکی",
        "got": "𐌲����𐌹��𐌺 	",
	"gn": "Avañe'ẽ",         # Guarani
	"g": "ગુજરાતી",          # Gujarati
	"gv": "Gaelg",           # Manx

	"ha": "هَوُسَ",          # Hausa
        "hak": "Hak-kâ-fa / 客家話",
        "haw": "Hawai`i",
	"he": "עברית" ,          # Hebrew (formerly iw)
	"hi": "हिन्दी" ,   # Hindi
        "hif": "Fiji Hindi",
	"ho": "Hiri Mot",        # Hiri Motu
	"hr": "Hrvatski" ,       # Croatian
        "hsb": "Hornjoserbsce",
	"ht": "Krèyol ayisyen" , # Haitian; Haitian Creole
	"hu": "Magyar" ,         # Hungarian
	"hy": "Հայերեն",         # Armenian
	"hz": "Otsiherero",      # Herero

        "ia": "Interlingua",
        "ie": "Interlingue",
	"id": "Bahasa Indonesia",# Indonesian (formerly in)
	"ig": "Igbo",            # Igbo
	"ii": "ꆇꉙ 	",       # Sichuan Yi
	"ik": "Iñupiak",         # Inupiak
        "ilo": "Ilokano",
        "io": "Ido" ,
	"is": "Íslenska" ,       # Icelandic
	"it": "Italiano" ,       # Italian
	"i": "ᐃᓄᒃᑎᑐᑦ",           # Inuktitut

	"ja": "日本語",          # Japanese
        "jbo": "Lojban",
	"jv": "Basa Jawa",       # Javanese

	"ka": "ქართული" ,        # Georgian
        "kaa": "Qaraqalpaqsha",
        "kab": "Taqbaylit",
	"kg": "KiKongo",         # Kongo
	"ki": "Gĩkũyũ",          # Kikuyu
	"kj": "Kuanyama",        # Kuanyama
	"kk": "Қазақша",         # Kazakh
	"kl": "Kalaallisut",     # Kalaallisut; Greenlandic
	"km": "ភាសាខ្មែរ",       # Khmer; Cambodian
	"kn": "ಕನ್ನಡ",           # Kannada
	"ko": "한국어" ,         # Korean
	"kr": "Kanuri",          # Kanuri
	"ks": "कश्मीरी / كشميري", # Kashmiri
        "ksh": "Ripoarisch",
	"ku": "Kurdî / كوردی",   # Kurdish
	"kv": "Коми",            # Komi
	"kw": "Kernewek/Karnuack", # Cornish
	"ky": "Кыргызча",        # Kirghiz

	"la": "Latina" ,         # Latin
        "lad": "Dzhudezmo",
	"lb": "Lëtzebuergesch" , # Letzeburgesch
        "lbe": "Лакку",
	"lg": "Luganda",         # Ganda
	"li": "Limburgs",        # Limburgish; Limburger; Limburgan
        "lij": "Lígur", 
	"ln": "Lingala", 	 # Lingala
        "lmo": "Lumbaart",
	"lo": "ລາວ",             # Lao; Laotian
	"lt": "Lietuvių" ,       # Lithuanian
	"lua": "Luba",           # Luba
	"lv": "Latvieš" ,        # Latvian; Lettish

        "map-bms": "Basa Banyumasan",
        "mdf": "Мокшень (Mokshanj Kälj)",
	"mg": "Malagasy",        # Malagasy
	"mh": "Ebon",            # Marshall
	"mi": "Māori",           # Maori
	"mk": "Македонски" ,     # Macedonian
	"ml": None,              # Malayalam
	"mn": "Монгол",          # Mongolian
	"mo": "Молдовеняскэ",    # Moldavian
	"mr": "मराठी" ,     # Marathi
	"ms": "Bahasa Melay" ,   # Malay
	"mt": "Malti",           # Maltese
        "mus": "Muskogee",
	"my": "မ္ရန္‌မာစာ",      # Burmese
        "myv": "Эрзянь (Erzjanj Kelj)",
        "mzn": "مَزِروني",

	"na": "dorerin Naoero",  # Nauru
        "nah": "Nāhuatl",
        "nap": "Nnapulitano",
	"nb": "Norsk (Bokmål)",  # Norwegian Bokm@aa{}l
	"nd": None,              # Ndebele, North
        "nds": "Plattdüütsch",
        "nds-nl": "Nedersaksisch",
	"ne": "नेपाली",    # Nepali
        "new": "नेपाल भाषा" , # Nepal Bhasa
	"ng": "Oshiwambo",       # Ndonga
	"nl": "Nederlands" ,     # Dutch
	"nn": "Nynorsk",         # Norwegian Nynorsk
	"no": "Norsk (Bokmål)" , # Norwegian
        "nov": "Novial",
	"nr": None,              # Ndebele, South
        "nrm": "Nouormand/Normaund",
	"nv": "Diné bizaad",     # Navajo
	"ny": "Chi-Chewa",       # Chichewa; Nyanja

	"oc": "Occitan",         # Occitan; Proven@,{c}al
	"oj": None,              # Ojibwa
	"om": "Oromoo",          # (Afan) Oromo
	"or": "ଓଡ଼ିଆ",           # Oriya
	"os": "Иронау",          # Ossetian; Ossetic

	"pa": "ਪੰਜਾਬੀ" ,         # Panjabi; Punjabi
        "pag": "Pangasinan",
        "pam": "Kapampangan",
        "pap": "Papiament",
        "pdc": "Deitsch",
	"pi": "पाऴि",        # Pali
        "pih": "Norfuk",
	"pl": "Polski" ,         # Polish
        "pms": "Piemontèis" ,
	"ps": "پښتو",            # Pashto, Pushto
	"pt": "Português" ,      # Portuguese

	"q": "Runa Simi" ,       # Quechua

	"rm": "Rumantsch",       # Rhaeto-Romance
        "rmy": "romani - रोमानी",
	"rn": "Kirundi",         # Rundi; Kirundi
	"ro": "Română" ,         # Romanian
        "roa-rup": "Armãneashce",
        "roa-tara": "Tarandíne",
	"ru": "Русский" ,        # Russian
	"rw": "Ikinyarwanda",    # Kinyarwanda

	"sa": "संस्कृतम्", # Sanskrit
        "sah": "Саха тыла (Saxa Tyla)",
	"sc": "Sardu",           # Sardinian
        "scn": "Sicilian", 
        "sco": "Scots",
	"sd": "سنڌي، سندھی ، सिन्ध", # Sindhi
	"se": "Sámegiella",      # Northern Sami
	"sg": "Sängö",           # Sango; Sangro
        "sh": "Srpskohrvatski / Српскохрватски" ,
        "si": "සිංහල",
        "simple": "Simple English" ,
	"sk": "Slovenčina" ,     # Slovak
	"sl": "Slovenščina" ,    # Slovenian
	"sm": "Gagana Samoa",    # Samoan
	"sn": "chiShona",        # Shona
	"so": "Soomaaliga",      # Somali
	"sr": "Српски / Srpski", # Serbian
        "srn": "Sranantongo",
	"ss": "SiSwati",         # Swati; Siswati
	"st": "Sesotho",         # Sesotho; Sotho, Southern
        "stk": "Seeltersk",
	"s": "Basa Sunda",       # Sundanese
        "sq": "Shqip" ,          # Albanian
        "szl": "Ślůnski",
	"sv": "Svenska" ,        # Swedish
	"sw": "Kiswahili",       # Swahili 

	"ta": "தமிழ்" ,          # Tamil
	"te": "తెలుగు" ,         # Telugu
        "tet": "Tetun",
	"tg": "Тоҷикӣ",          # Tajik
	"th": "ไทย" ,            # Thai
	"ti": "ትግርኛ",            # Tigrinya
	"tk": "تركمن / Туркмен", # Turkmen
	"tl": "Tagalog" ,        # Tagalog
	"tn": "Setswana",        # Tswana; Setswana
	"to": "faka Tonga",      # Tonga (?) # Also ZW ; MW
        "tokipona": "Tokipona",
        "tpi": "Tok Pisin",
 	"tr": "Türkçe" ,         # Turkish
	"ts": "Xitsonga",        # Tsonga
	"tt": "Tatarça / Татарча", # Tatar
        "tum": "chiTumbuka",
	"tw": "Twi",             # Twi
	"ty": "Reo Mā`ohi",      # Tahitian

        "udm": "Удмурт кыл",
	"ug": "Oyghurque",       # Uighur
	"uk": "Українська" ,     # Ukrainian
	"ur": "اردو",            # Urdu
	"uz": "O‘zbek",          # Uzbek

	"ve": "Tshivenda",       # Venda
        "vec": "Vèneto",
	"vi": "Tiếng Việt" ,     # Vietnamese
        "vls": "West-Vlams",
        "vo": "Volapük" ,
        
	"wa": "Walon",           # Walloon
        "war": "Winaray",
	"wo": "Wolof",           # Wolof
        "w": "吴语",

        "xal": "Хальмг",
	"xh": "isiXhosa",        # Xhosa

	"yi": "ייִדיש",          # Yiddish 
	"yo": "Yorùbá",          # Yoruba

	"za": "Cuengh",          # Zhuang
        "zea": "Zeêuws",
	"zh": "中文" ,           # Chinese
        "zh-classical": "古文 / 文言文",
        "zm-min-nan": "Bân-lâm-gú",
        "zh-yue": "粵語",
	"zu": "isiZulu"          # Zulu
    }

        

    
            
        

    
        
