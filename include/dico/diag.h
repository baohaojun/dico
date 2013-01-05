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

#ifndef __dico_diag_h
#define __dico_diag_h

#include <dico/types.h>

#define L_DEBUG     0
#define L_INFO      1
#define L_NOTICE    2
#define L_WARN      3
#define L_ERR       4
#define L_CRIT      5
#define L_ALERT     6
#define L_EMERG     7

#define L_MASK      0xff

#define L_CONS      0x8000

extern const char *dico_program_name;
extern const char *dico_invocation_name;
void dico_set_program_name(char *name);

typedef void (*dico_log_printer_t) (int /* lvl */,
			            int /* exitcode */,
				    int /* errcode */,
				    const char * /* fmt */,
				    va_list);
void _dico_stderr_log_printer(int, int, int, const char *, va_list);

void dico_set_log_printer(dico_log_printer_t prt);
void dico_vlog(int lvl, int errcode, const char *fmt, va_list ap);
void dico_log(int lvl, int errcode, const char *fmt, ...)
    DICO_PRINTFLIKE(3,4);
void dico_die(int exitcode, int lvl, int errcode, const char *fmt, ...)
    DICO_PRINTFLIKE(4,5);
int dico_str_to_diag_level(const char *str);

dico_stream_t dico_log_stream_create(int level);
dico_stream_t dico_dbg_stream_create(void);

#define DICO_DEBUG_SINFO(str)						\
    do {								\
	unsigned n = __LINE__;						\
	dico_stream_ioctl(str, DICO_DBG_CTL_SET_FILE, __FILE__);	\
	dico_stream_ioctl(str, DICO_DBG_CTL_SET_LINE, &n);		\
    } while (0)
    
#endif
