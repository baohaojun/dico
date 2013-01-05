/* This file is part of GNU Dico.
   Copyright (C) 2012 Sergey Poznyakoff

   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "gcide.h"

char gcide_webchr[256][4] = {
/* 00 */ "",
/* 01 */ "",
/* 02 */ "",
/* 03 */ "",
/* 04 */ "",
/* 05 */ "",
/* 06 */ "",
/* 07 */ "",
/* 08 */ "",
/* 09 */ "",
/* 0a */ "",
/* 0b */ "",
/* 0c */ "",
/* 0d */ "",
/* 0e */ "",
/* 0f */ "",
/* 10 */ "",
/* 11 */ "",
/* 12 */ "",
/* 13 */ "",
/* 14 */ "",
/* 15 */ "",
/* 16 */ "",
/* 17 */ "",
/* 18 */ "",
/* 19 */ "",
/* 1a */ "",
/* 1b */ "",
/* 1c */ "",
/* 1d */ "",
/* 1e */ "",
/* 1f */ "",
/* 20 */ "",
/* 21 */ "",
/* 22 */ "",
/* 23 */ "",
/* 24 */ "",
/* 25 */ "",
/* 26 */ "",
/* 27 */ "",
/* 28 */ "",
/* 29 */ "",
/* 2a */ "",
/* 2b */ "",
/* 2c */ "",
/* 2d */ "",
/* 2e */ "",
/* 2f */ "",
/* 30 */ "",
/* 31 */ "",
/* 32 */ "",
/* 33 */ "",
/* 34 */ "",
/* 35 */ "",
/* 36 */ "",
/* 37 */ "",
/* 38 */ "",
/* 39 */ "",
/* 3a */ "",
/* 3b */ "",
/* 3c */ "<",
/* 3d */ "",
/* 3e */ ">",
/* 3f */ "",
/* 40 */ "",
/* 41 */ "",
/* 42 */ "",
/* 43 */ "",
/* 44 */ "",
/* 45 */ "",
/* 46 */ "",
/* 47 */ "",
/* 48 */ "",
/* 49 */ "",
/* 4a */ "",
/* 4b */ "",
/* 4c */ "",
/* 4d */ "",
/* 4e */ "",
/* 4f */ "",
/* 50 */ "",
/* 51 */ "",
/* 52 */ "",
/* 53 */ "",
/* 54 */ "",
/* 55 */ "",
/* 56 */ "",
/* 57 */ "",
/* 58 */ "",
/* 59 */ "",
/* 5a */ "",
/* 5b */ "",
/* 5c */ "",
/* 5d */ "",
/* 5e */ "",
/* 5f */ "",
/* 60 */ "",
/* 61 */ "",
/* 62 */ "",
/* 63 */ "",
/* 64 */ "",
/* 65 */ "",
/* 66 */ "",
/* 67 */ "",
/* 68 */ "",
/* 69 */ "",
/* 6a */ "",
/* 6b */ "",
/* 6c */ "",
/* 6d */ "",
/* 6e */ "",
/* 6f */ "",
/* 70 */ "",
/* 71 */ "",
/* 72 */ "",
/* 73 */ "",
/* 74 */ "",
/* 75 */ "",
/* 76 */ "",
/* 77 */ "",
/* 78 */ "",
/* 79 */ "",
/* 7a */ "",
/* 7b */ "",
/* 7c */ "",
/* 7d */ "",
/* 7e */ "",
/* 7f */ "",
/* 80 */ "Ç",
/* 81 */ "ü",
/* 82 */ "é",
/* 83 */ "â",
/* 84 */ "ä",
/* 85 */ "à",
/* 86 */ "å",
/* 87 */ "ç",
/* 88 */ "ê",
/* 89 */ "ë",
/* 8a */ "è",
/* 8b */ "ï",
/* 8c */ "î",
/* 8d */ "ì",
/* 8e */ "Ä",
/* 8f */ "Å",
/* 90 */ "È",
/* 91 */ "æ",
/* 92 */ "Æ",
/* 93 */ "ô",
/* 94 */ "ö",
/* 95 */ "ò",
/* 96 */ "û",
/* 97 */ "ù",
/* 98 */ "ÿ",
/* 99 */ "Ö",
/* 9a */ "Ü",
/* 9b */ "£",
/* 9c */ "",
/* 9d */ "",
/* 9e */ "",
/* 9f */ "",
/* a0 */ "á",
/* a1 */ "í",
/* a2 */ "ó",
/* a3 */ "ú",
/* a4 */ "ñ",
/* a5 */ "Ñ",
/* a6 */ "⅔",
/* a7 */ "⅓",
/* a8 */ "",
/* a9 */ "˝",
/* aa */ "",
/* ab */ "½",
/* ac */ "¼",
/* ad */ "",
/* ae */ "",
/* af */ "",
/* b0 */ "<?>", /* Place-holder for unknown or illegible character. */
/* b1 */ "",
/* b2 */ "",
/* b3 */ "",
/* b4 */ "↑",
/* b5 */ "☞",   /* pointing hand (printer's "fist") */
/* b6 */ "˝",   /* bold accent */
/* b7 */ "´",   /* light accent */ 
/* b8 */ "”",
/* b9 */ "",
/* ba */ "‖",
/* bb */ "",
/* bc */ "§",
/* bd */ "“",
/* be */ "ā",
/* bf */ "‘",
/* c0 */ "ṉ",   /* "n sub-macron" */
/* c1 */ "♯",
/* c2 */ "♭",
/* c3 */ "–",
/* c4 */ "―",
/* c5 */ "t",   /* First part of th ligature */
/* c6 */ "ī",
/* c7 */ "ē",
/* c8 */ "ḍ",   /* Sanskrit/Tamil d dot */
/* c9 */ "ṇ",   /* Sanskrit/Tamil n dot */
/* ca */ "ṭ",   /* Sanskrit/Tamil t dot */
/* cb */ "ĕ",
/* cc */ "ĭ",
/* cd */ "",
/* ce */ "ŏ",
/* cf */ "‐",   /* short dash */
/* d0 */ "—",
/* d1 */ "Œ",
/* d2 */ "œ",
/* d3 */ "ō",
/* d4 */ "ū",
/* d5 */ "ǒ",
/* d6 */ "ǣ",
/* d7 */ "ōē",  /* FIXME: oe ligature with macron */
/* d8 */ "‖",
/* d9 */ "",
/* da */ "",
/* db */ "",
/* dc */ "ŭ",
/* dd */ "ă",
/* de */ "˘",   /* FIXME: crescent: 1F319? */
/* df */ "ȳ",
/* e0 */ "a",   /* FIXME: a "semilong" (has a macron above with a short */
                /* vertical bar on top the center of the macron) */
/* e1 */ "e",   /* FIXME: e "semilong" */ 
/* e2 */ "i",   /* FIXME: i "semilong" */
/* e3 */ "o",   /* FIXME: o "semilong" */
/* e4 */ "u",   /* FIXME: u "semilong" */
/* e5 */ "ȧ",   /* a with dot above */
/* e6 */ "μ",   
/* e7 */ "h",   /* second part of th ligature */
/* e8 */ "",
/* e9 */ "",
/* ea */ "",
/* eb */ "ð",
/* ec */ "",
/* ed */ "þ",
/* ee */ "ã",
/* ef */ "ṅ",
/* f0 */ "ṛ",
/* f1 */ "",
/* f2 */ "",
/* f3 */ "",
/* f4 */ "ȝ",
/* f5 */ "—",
/* f6 */ "÷",
/* f7 */ "≈",
/* f8 */ "°",
/* f9 */ "•",
/* fa */ "·",
/* fb */ "√",
/* fc */ "",
/* fd */ "",
/* fe */ "",
/* ff */ "",
};

static char xdig[] = "0123456789abcdef";
#define xnum(c) (strchr(xdig, (c)) - xdig)

char const *
gcide_escape_to_utf8(const char *esc)
{
    char const *s = gcide_webchr[xnum(esc[0]) * 16 + xnum(esc[1])];
    return s[0] ? s : NULL;
}


/* Local Variables: */
/* mode: c */
/* eval: (setq buffer-file-coding-system (if window-system 'utf-8 'raw-text-unix)) */
/* End: */
