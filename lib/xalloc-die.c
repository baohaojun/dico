/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

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

#include <xdico.h>
#include <sysexits.h>

void
xalloc_die()
{
    dico_die(EX_OSERR, L_CRIT, 0, "Not enough memory");
}