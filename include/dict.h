#ifndef __dict_h
#define __dict_h

#ifndef INPUTDICTPATH
# define INPUTDICTPATH "/usr/local/lib"
#endif
#ifndef DICTPATH
# define DICTPATH "/usr/local/lib/jdict"
#endif

#define DICT_DB "dict.db"
#define JIS_INDEX_NAME "jis.db"
#define UNICODE_INDEX_NAME "unicode.db"
#define CORNER_INDEX_NAME "corner.db"
#define FREQ_INDEX_NAME "freq.db"
#define NELSON_INDEX_NAME "nelson.db"
#define HALPERN_INDEX_NAME "halpern.db"
#define GRADE_INDEX_NAME "grade.db"
#define PINYIN_INDEX_NAME "pinyin.db"
#define ENGLISH_INDEX_NAME "english.db"
#define KANJI_INDEX_NAME "kanji.db"
#define WORDS_INDEX_NAME "words.db"
#define BUSHU_INDEX_NAME "bushu.db"
#define SKIP_INDEX_NAME "skip.db"
#define XREF_INDEX_NAME "xref.db"
#define YOMI_INDEX_NAME "yomi.db"
#define ROMAJI_INDEX_NAME "romaji.db"

typedef unsigned char Uchar;
typedef unsigned short Ushort;
typedef unsigned int Uint;

enum index_id {
    index_jis,
    index_unicode,
    index_corner,
    index_freq,
    index_nelson,
    index_halpern,
    index_grade,
    index_bushu,
    index_skip,
    index_pinyin,
    index_english,
    index_kanji,
    index_xref,
    index_words,
    index_yomi,
    index_romaji,

    MAX_INDEX
};

typedef struct {
    Ushort bushu;        /* The radical number */
    Ushort numstrokes;   /* The number of strokes */
} Bushu;
    
typedef struct {
    Ushort kanji;
    Ushort pos;
} Xref;

/* Dictionary entry structure.
 */
 
typedef struct translation {
    Bushu  bushu;
    Ushort Qindex;		/* for the "four corner" lookup method */
    unsigned skip;              /* SKIP code */
    Ushort Jindex;              /* jis index */
    Ushort Uindex;		/* "Unicode" index */
    Ushort Nindex;		/* Nelson dictionary */
    Ushort Hindex;		/* Halpern dictionary */

    Ushort frequency;		/* frequency that kanji is used */
    Uchar grade_level;		/* akin to  school class level */

    int refcnt;                 /* Number of references to this kanji */ 
    
    /* Offsets of the string values.  These point to the memory after
       DictEntry structure (see DICT_.*_PTR macros, below).  
       Offset 0 means there is no string.
       All strings are zero-terminated (16-bit terminate with two 
       zeroes. */
    
    size_t english;		/* english translation string. */
    size_t yomi;	        /* kana, actually */
    size_t kanji;		/* can be pointer to pronunciation */
    size_t pinyin;              /* Pinyin pronunciation */
} DictEntry;

#define DICT_PTR(e, f) ((char*)((e) + 1) + (e)->f)
#define DICT_ENGLISH_PTR(e) DICT_PTR(e, english)
#define DICT_PINYIN_PTR(e)  DICT_PTR(e, pinyin)
#define DICT_KANJI_PTR(e)   DICT_PTR(e, kanji)
#define DICT_YOMI_PTR(e)    DICT_PTR(e, yomi)

typedef struct {
    int numkanji;
    int numentries;
    Ushort lowestkanji;
    Ushort highestkanji;
    int maxlen8;
    int maxlen16;
} Dictheader;

#endif
