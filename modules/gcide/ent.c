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

struct gcide_entity {
    char *ent;
    char *text;
};

static struct gcide_entity gcide_entity[] = {
    { "Cced",   "Ç" },
    { "uum",    "ü" },
    { "eacute", "é" },
    { "acir",   "â" },
    { "aum",    "ä" },
    { "agrave", "à" },
    { "aring",  "å" },
    { "ccedil", "ç" },
    { "cced",   "ç" },
    { "ecir",   "ê" },
    { "eum",    "ë" },
    { "egrave", "è" },
    { "ium",    "ï" },
    { "icir",   "î" },
    { "igrave", "ì" },
    { "Aum",    "Ä" },
    { "Aring",  "Å" },
    { "Eacute", "È" },
    { "ae",     "æ" },
    { "AE",     "Æ" },
    { "ocir",   "ô" },
    { "oum",    "ö" },
    { "ograve", "ò" },
    { "oacute", "ó" }, 
    { "Oacute", "Ó" }, 
    { "ucir",   "û" },
    { "ugrave", "ù" },
    { "uacute", "ú" },
    { "yum",    "ÿ" },
    { "Oum",    "Ö" },
    { "Uum",    "Ü" },
    { "pound",  "£", },
    { "aacute", "á" },
    { "iacute", "í" },
    { "frac23", "⅔" },
    { "frac13", "⅓" },
    { "frac12", "½" },
    { "frac14", "¼" },
    { "?",      "<?>" }, /* Place-holder for unknown or illegible character. */
    { "hand",   "☞" },   /* pointing hand (printer's "fist") */
    { "sect",   "§" },
    { "amac",   "ā" },
    { "nsm",    "ṉ" },   /* "n sub-macron" */
    { "sharp",  "♯" },
    { "flat",   "♭" },
    { "th",     "th" },
    { "imac",   "ī" },
    { "emac",   "ē" },
    { "dsdot",  "ḍ" },   /* Sanskrit/Tamil d dot */
    { "nsdot",  "ṇ" },   /* Sanskrit/Tamil n dot */
    { "tsdot",  "ṭ" },   /* Sanskrit/Tamil t dot */
    { "ecr",    "ĕ" },
    { "icr",    "ĭ" },
    { "ocr",    "ŏ" },
    { "OE",     "Œ" },
    { "oe",     "œ" },
    { "omac",   "ō" },
    { "umac",   "ū" },
    { "ocar",   "ǒ" },
    { "aemac",  "ǣ" },
    { "ucr",    "ŭ" },
    { "acr",    "ă" },
    { "ymac",   "ȳ" },
    { "asl",    "a" },   /* FIXME: a "semilong" (has a macron above with a short */
    { "esl",    "e" },   /* FIXME: e "semilong" */ 
    { "isl",    "i" },   /* FIXME: i "semilong" */
    { "osl",    "o" },   /* FIXME: o "semilong" */
    { "usl",    "u" },   /* FIXME: u "semilong" */
    { "adot",   "ȧ" },   /* a with dot above */
    { "edh",    "ð" },
    { "thorn",  "þ" },
    { "atil",   "ã" },
    { "etil",   "ẽ" },
    { "itil",   "ĩ" },
    { "otil",   "õ" },
    { "util",   "ũ" },
    { "ntil",   "ñ" },
    { "Atil",   "Ã" },
    { "Etil",   "Ẽ" },
    { "Itil",   "Ĩ" },
    { "Otil",   "Õ" },
    { "Util",   "Ũ" },
    { "Ntil",   "Ñ" },
    { "ndot",   "ṅ" },
    { "rsdot",  "ṛ" },
    { "yogh",   "ȝ" },
    { "deg",    "°" },
    { "middot", "•" },
    { "root",   "√" },
    /* Greek alphabet */
    { "alpha",    "α" },
    { "beta",     "β" },	
    { "gamma",    "γ" },	
    { "delta",    "δ" },	
    { "epsilon",  "ε" }, 
    { "zeta",     "ζ" },	
    { "eta",      "η" },	
    { "theta",    "θ" }, 
    { "iota",     "ι" },  
    { "kappa",    "κ" }, 
    { "lambda",   "λ" },
    { "mu",       "μ" },    
    { "nu",       "ν" },    
    { "xi",       "ξ" },    
    { "omicron",  "ο" },
    { "pi",       "π" }, 
    { "rho",      "ρ" }, 
    { "sigma",    "σ" }, 
    { "sigmat",   "ς" }, 
    { "tau",      "τ" },	
    { "upsilon",  "υ" }, 
    { "phi",      "φ" }, 
    { "chi",      "χ" }, 
    { "psi",      "ψ" }, 
    { "omega",    "ω" },
    { "digamma",  "ϝ" },
    { "ALPHA",    "Α" },
    { "BETA",     "Β" },	
    { "GAMMA",    "Γ" },	
    { "DELTA",    "Δ" },	
    { "EPSILON",  "Ε" }, 
    { "ZETA",     "Ζ" },	
    { "ETA",      "Η" },	
    { "THETA",    "Θ" }, 
    { "IOTA",     "Ι" },  
    { "KAPPA",    "Κ" }, 
    { "LAMBDA",   "Λ" },
    { "MU",       "Μ" },    
    { "NU",       "Ν" },    
    { "XI",       "Ξ" },    
    { "OMICRON",  "Ο" },
    { "PI",       "Π" }, 
    { "RHO",      "Ρ" }, 
    { "SIGMA",    "Σ" }, 
    { "TAU",      "Τ" },	
    { "UPSILON",  "Υ" }, 
    { "PHI",      "Φ" }, 
    { "CHI",      "Χ" }, 
    { "PSI",      "Ψ" }, 
    { "OMEGA",    "Ω" },
    /* Italic letters */
    { "AIT",      "A" },
    { "BIT",      "B" },
    { "CIT",      "C" },
    { "DIT",      "D" },
    { "EIT",      "E" },
    { "FIT",      "F" },
    { "GIT",      "G" },
    { "HIT",      "H" },
    { "IIT",      "I" },
    { "JIT",      "J" },
    { "KIT",      "K" },
    { "LIT",      "L" },
    { "MIT",      "M" },
    { "NOT",      "N" },
    { "OIT",      "O" },
    { "PIT",      "P" },
    { "QIT",      "Q" },
    { "RIT",      "R" },
    { "SIT",      "S" },
    { "TIT",      "T" },
    { "UIT",      "U" },
    { "VIT",      "V" },
    { "WIT",      "W" },
    { "XIT",      "X" },
    { "YIT",      "Y" },
    { "ZIT",      "Z" },    
    { "ait",      "a" },
    { "bit",      "b" },
    { "cit",      "c" },
    { "dit",      "d" },
    { "eit",      "e" },
    { "fit",      "f" },
    { "git",      "g" },
    { "hit",      "h" },
    { "iit",      "i" },
    { "jit",      "j" },
    { "kit",      "k" },
    { "lit",      "l" },
    { "mit",      "m" },
    { "not",      "n" },
    { "oit",      "o" },
    { "pit",      "p" },
    { "qit",      "q" },
    { "rit",      "r" },
    { "sit",      "s" },
    { "tit",      "t" },
    { "uit",      "u" },
    { "vit",      "v" },
    { "wit",      "w" },
    { "xit",      "x" },
    { "yit",      "y" },
    { "zit",      "z" },
    /* FIXME: Vowels with a double dot below. There's nothing suitable in the Unicode */
    { "add",      "a" }, 
    { "udd",      "u" },
    { "ADD",      "A" }, 
    { "UDD",      "U" },

    /* Accents */
    { "prime",    "´" },
    { "bprime",   "˝" },
    { "mdash",    "—" },
    { "divide",   "÷" },
    
    /* Quotes */
    { "lsquo",    "‘" },
    { "ldquo",    "“" },
    { "rdquo",    "”" },

    { "dagger",   "†" },
    { "dag",      "†" },
    { "Dagger",   "‡" },
    { "ddag",     "‡" },
    { "para",     "§" },
    { "gt",       ">" },
    { "lt",       "<" },
    { "rarr",     "→" },
    { "larr",     "←" },
    { "schwa",    "ə" },
      
    { "br",       "\n" },
    { "and",      "and" },
    { "or",       "or" },
    { "sec",      "˝" },
    { NULL }
};

char const *
gcide_entity_to_utf8(const char *str)
{
    struct gcide_entity *p;
    size_t len;

    if (str[0] == '<') {
	str++;
	len = strcspn(str, "/");
    } else
	len = strlen(str);

    for (p = gcide_entity; p->ent; p++) {
	if (p->ent[0] == str[0] && strlen(p->ent) == len && memcmp(p->ent, str, len) == 0)
	    return p->text;
    }
    return NULL;
}
