/* This file is part of GNU Dico
   Copyright (C) 2003,2004,2007,2008 Sergey Poznyakoff
  
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

struct itr_shell_command {
    const char *name;
    int minargc;
    int maxargc;
    void (*fun) (void *object, int argc, char **argv);
    char *argstr;
    char *docstr;
};

struct itr_shell {
    void *object;
    dico_iterator_t (*get_iterator) (void *);
    void (*print_item) (void *);
    size_t (*count) (void *);
    struct itr_shell_command *cmdtab;
};

void shell(struct itr_shell *shp);
dico_iterator_t shell_iterator();
