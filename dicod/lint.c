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

#include <dicod.h>

struct cmdopt {
    const char **argv;
    int argc;
};

static int
add_i_option(int flag, const char *dir, void *data)
{
    struct cmdopt *cmdopt = data;
    cmdopt->argv[cmdopt->argc++] = "-I";
    cmdopt->argv[cmdopt->argc++] = dir;
    return 0;
}

void
run_lint()
{
    size_t n;
    struct cmdopt cmdopt;

    n = grecs_include_path_count(GRECS_USR_INCLUDE);
    cmdopt.argc = 10 + 2 * n;
    cmdopt.argv = xcalloc(cmdopt.argc + 1, sizeof cmdopt.argv[0]);
	
    cmdopt.argc = 0;
    cmdopt.argv[cmdopt.argc++] = dico_invocation_name;
    cmdopt.argv[cmdopt.argc++] = "--lint";
    cmdopt.argv[cmdopt.argc++] = "--preprocessor";
    cmdopt.argv[cmdopt.argc++] = grecs_preprocessor;
    if (log_to_stderr)
	cmdopt.argv[cmdopt.argc++] = "--stderr";
    else 
	dicod_log_encode_envar();
    grecs_foreach_include_dir(GRECS_USR_INCLUDE, add_i_option, &cmdopt);
    cmdopt.argv[cmdopt.argc++] = "--config";
    cmdopt.argv[cmdopt.argc++] = config_file;
    if (debug_level) {
	cmdopt.argv[cmdopt.argc++] = "--debug";
	cmdopt.argv[cmdopt.argc++] = debug_level_str;
    }
    cmdopt.argv[cmdopt.argc] = NULL;
    
    execv(cmdopt.argv[0], (char**) cmdopt.argv);
}

