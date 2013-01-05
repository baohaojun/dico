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

#include <dicod.h>

struct dico_stat current_stat, total_stat;

void
clear_stats()
{
    total_stat.defines += current_stat.defines;
    total_stat.matches += current_stat.matches;
    total_stat.compares += current_stat.compares;
    memset(&current_stat, 0, sizeof(current_stat));
}

void
begin_timing(const char *name)
{
    if (timing_option) {
	timer_start(name);
	clear_stats();
    }
}

void
report_timing(dico_stream_t stream, xdico_timer_t t, struct dico_stat *sp)
{
    dico_stream_write(stream, " [", 2);
    if (sp)
	stream_printf(stream, "d/m/c = %lu/%lu/%lu ",
		      (unsigned long) sp->defines,
		      (unsigned long) sp->matches,
		      (unsigned long) sp->compares);
    timer_format_time(stream, timer_get_real(t));
    dico_stream_write(stream, "r ", 2);
    timer_format_time(stream, timer_get_user(t));
    dico_stream_write(stream, "u ", 2);
    timer_format_time(stream, timer_get_system(t));
    dico_stream_write(stream, "s]", 2);
}

void
report_current_timing(dico_stream_t stream, const char *name)
{
    if (timing_option) {
	xdico_timer_t t = timer_stop(name);
	report_timing(stream, t, &current_stat);
	clear_stats();
    }
}

    
