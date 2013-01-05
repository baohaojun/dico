/* This file is part of GNU Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

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
#include <dico.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#ifdef USE_LIBZ
# include <zlib.h>
#endif
#include <appi18n.h>

#define DICTORG_ENTRY_PREFIX        "00-database"
#define DICTORG_ENTRY_PREFIX_LEN    sizeof(DICTORG_ENTRY_PREFIX)-1
#define DICTORG_SHORT_ENTRY_NAME    DICTORG_ENTRY_PREFIX"-short"
#define DICTORG_LONG_ENTRY_NAME     DICTORG_ENTRY_PREFIX"-long"
#define DICTORG_INFO_ENTRY_NAME     DICTORG_ENTRY_PREFIX"-info"

#define DICTORG_FLAG_UTF8           DICTORG_ENTRY_PREFIX"-utf8"
#define DICTORG_FLAG_8BIT_NEW       DICTORG_ENTRY_PREFIX"-8bit-new"
#define DICTORG_FLAG_8BIT_OLD       DICTORG_ENTRY_PREFIX"-8bit"
#define DICTORG_FLAG_ALLCHARS       DICTORG_ENTRY_PREFIX"-allchars"
#define DICTORG_FLAG_VIRTUAL        DICTORG_ENTRY_PREFIX"-virtual"
#define DICTORG_FLAG_ALPHABET       DICTORG_ENTRY_PREFIX"-alphabet"
#define DICTORG_FLAG_DEFAULT_STRAT  DICTORG_ENTRY_PREFIX"-default-strategy"

#define DICTORG_ENTRY_PLUGIN        DICTORG_ENTRY_PREFIX"-plugin"
#define DICTORG_ENTRY_PLUGIN_DATA   DICTORG_ENTRY_PREFIX"-plugin-data"

#define DICTORG_UNKNOWN    0
#define DICTORG_TEXT       1
#define DICTORG_GZIP       2
#define DICTORG_DZIP       3

/* Header field definitions from dict.org project */

/* For gzip-compatible header, as defined in RFC 1952 */

				/* Magic for GZIP (rfc1952)                */
#define GZ_MAGIC1     0x1f	/* First magic byte                        */
#define GZ_MAGIC2     0x8b	/* Second magic byte                       */

				/* FLaGs (bitmapped), from rfc1952         */
#define GZ_FTEXT      0x01	/* Set for ASCII text                      */
#define GZ_FHCRC      0x02	/* Header CRC16                            */
#define GZ_FEXTRA     0x04	/* Optional field (random access index)    */
#define GZ_FNAME      0x08	/* Original name                           */
#define GZ_COMMENT    0x10	/* Zero-terminated, human-readable comment */
#define GZ_MAX           2	/* Maximum compression                     */
#define GZ_FAST          4	/* Fasted compression                      */

				/* These are from rfc1952                  */
#define GZ_OS_FAT        0	/* FAT filesystem (MS-DOS, OS/2, NT/Win32) */
#define GZ_OS_AMIGA      1	/* Amiga                                   */
#define GZ_OS_VMS        2	/* VMS (or OpenVMS)                        */
#define GZ_OS_UNIX       3      /* Unix                                    */
#define GZ_OS_VMCMS      4      /* VM/CMS                                  */
#define GZ_OS_ATARI      5      /* Atari TOS                               */
#define GZ_OS_HPFS       6      /* HPFS filesystem (OS/2, NT)              */
#define GZ_OS_MAC        7      /* Macintosh                               */
#define GZ_OS_Z          8      /* Z-System                                */
#define GZ_OS_CPM        9      /* CP/M                                    */
#define GZ_OS_TOPS20    10      /* TOPS-20                                 */
#define GZ_OS_NTFS      11      /* NTFS filesystem (NT)                    */
#define GZ_OS_QDOS      12      /* QDOS                                    */
#define GZ_OS_ACORN     13      /* Acorn RISCOS                            */
#define GZ_OS_UNKNOWN  255      /* unknown                                 */

#define GZ_RND_S1       'R'	/* First magic for random access format    */
#define GZ_RND_S2       'A'	/* Second magic for random access format   */

#define GZ_ID1           0	/* GZ_MAGIC1                               */
#define GZ_ID2           1	/* GZ_MAGIC2                               */
#define GZ_CM            2	/* Compression Method (Z_DEFALTED)         */
#define GZ_FLG	         3	/* FLaGs (see above)                       */
#define GZ_MTIME         4	/* Modification TIME                       */
#define GZ_XFL           8	/* eXtra FLags (GZ_MAX or GZ_FAST)         */
#define GZ_OS            9	/* Operating System                        */
#define GZ_XLEN         10	/* eXtra LENgth (16bit)                    */
#define GZ_FEXTRA_START 12	/* Start of extra fields                   */
#define GZ_SI1          12	/* Subfield ID1                            */
#define GZ_SI2          13      /* Subfield ID2                            */
#define GZ_SUBLEN       14	/* Subfield length (16bit)                 */
#define GZ_VERSION      16      /* Version for subfield format             */
#define GZ_CHUNKLEN     18	/* Chunk length (16bit)                    */
#define GZ_CHUNKCNT     20	/* Number of chunks (16bit)                */
#define GZ_RNDDATA      22	/* Random access data (16bit)              */

struct index_entry {
    char *word;             /* Word */
    size_t length;          /* Its length in bytes */
    size_t wordlen;         /* Its length in characters */
    off_t offset;           /* Offset of the corresponding article in file */
    size_t size;            /* Size of the article */
};

struct rev_entry {
    char *word;       
    struct index_entry *ptr;
};
    
struct dictdb {
    const char *dbname;
    char *basename;
    size_t numwords;
    struct index_entry *index;
    struct rev_entry *suf_index;
    int show_dictorg_entries;
    dico_stream_t stream;
};

enum result_type {
    result_match,
    result_define
};

struct result {
    struct dictdb *db;
    enum result_type type;
    size_t compare_count;
    dico_list_t list;
    dico_iterator_t itr;
};

typedef int (*entry_match_t) (struct dictdb *,
			      const char *word,
			      struct result *res);

struct strategy_def {
    struct dico_strategy strat;
    entry_match_t match;
};

dico_stream_t dict_stream_create(const char *filename, size_t cache_size);
