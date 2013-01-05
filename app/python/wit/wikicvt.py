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

import sys
import getopt
from wiki2html import *
from wiki2text import *

def usage(code=0):
    print """
usage: %s [-hvt] [-l lang] [-o kw=val] [--lang=lang] [--option kw=val]
          [--text] [--help] [--verbose] file
""" % (sys.argv[0])
    sys.exit(code)

def main():
    verbose_flag = 0
    html = 1
    lang = "pl"
    kwdict = {}
    debug = 0
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:hl:o:tv",
                                   ["debug=", "help", "lang=", "option=",
                                    "text", "input-text", "verbose" ])
    except getopt.GetoptError:
        usage(1)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
        elif o in ("-v", "--verbose"):
            verbose_flag = verbose_flag + 1
        elif o in ("-t", "--text"):
            html = 0
        elif o in ("-l", "--lang"):
            lang = a
        elif o in ("-o", "--option"):
            (kw,sep,val) = a.partition('=')
            if val != '':
                kwdict[kw] = eval(val)
        elif o == "--input-text":
            input_text = True
        elif o in ("-d", "--debug"):
            debug = eval(a)

    if len(args) == 1:
        if args[0] == '-':
            kwdict['file'] = sys.stdin
        else:
            kwdict['filename'] = args[0]
    else:
        usage(1)

    kwdict['lang']=lang        
    if html:
        markup = HtmlWiktionaryMarkup(**kwdict)
    else:
        markup = TextWiktionaryMarkup(**kwdict)
    markup.debug_level = debug
    markup.parse()
    print str(markup)
#    if verbose_flag > 0:
#        markup.output()

if __name__ == '__main__':
    main()            
