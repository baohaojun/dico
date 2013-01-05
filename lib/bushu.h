/* $Id$
 */

#define BushuError -1
#define BushuStrict 0
#define BushuAbsolute 1
#define BushuFuzzy 2

typedef struct {
    int type;
    int bushu;
    int numstrokes;
    int off;
} TempBushu;

int parse_bushu(char *, TempBushu *);
int decode_bushu(Matchdir, TempBushu *, Bushu *);
void format_bushu(char *, int /* radical */ , int /* strokes */);

extern int bushu_num[];
extern int bushu_stroke[];
extern int bushu_var_stroke[];
extern int bushu_index[];
extern int bushu_count[];
extern char bushu_string[][2];
extern int bushu_last;
extern int max_bushu_strokes;
extern int max_bushu_count;
