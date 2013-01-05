#  This file is part of GNU Dico.
#  Copyright (C) 2008-2010, 2012 Wojciech Polak
#
#  GNU Dico is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3, or (at your option)
#  any later version.
#
#  GNU Dico is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

import re
import os
import sys
import atexit
import getopt
import readline
import curses.ascii
import socket
import dicoclient

class Shell:
    """Simple GNU Dico-Python Shell."""

    prompt = 'dico> '
    prefix = '.'

    default_host = 'gnu.org.ua'
    database = '!'
    strategy = '.'
    last_matches = []
    last_databases = []
    last_strategies = []
    transcript = False

    def __init__ (self, opts, args):
        for o, a in opts:
            if o in ('-h', '--host'):
                self.default_host = a

        self.dc = dicoclient.DicoClient (self.default_host)

    def run (self):
        histfile = os.path.expanduser ('~/.dico_history')
        try:
            readline.read_history_file (histfile)
        except IOError:
            pass
        atexit.register (readline.write_history_file, histfile)

        print '\nType ? for help summary\n'
        while True:
            try:
                input = raw_input (self.prompt).strip ()
                input = unicode (input, 'utf_8')
            except (EOFError, KeyboardInterrupt):
                print
                sys.exit ()

            try:
                self.parse (input)
            except socket.timeout:
                self.__error ('socket timed out')
            except dicoclient.DicoNotConnectedError:
                try:
                    self.dc.open ()
                    dict = self.dc.show_databases ()
                    self.last_databases = dict['databases']
                    dict = self.dc.show_strategies ()
                    self.last_strategies = dict['strategies']
                    self.parse (input)
                except socket.error, (errno, strerror):
                    self.__error (strerror)

    def parse (self, input):
        if len (input) < 1:
            return
        if input[0] == self.prefix:
            self.parse_command (input[1:])
        elif input == '?':
            self.print_help ()
        elif re.match (r'^[0-9]+$', input):
            try:
                match = self.last_matches[int (input)]
                dict = self.dc.define (match[0], match[1])
                if 'count' in dict:
                    for d in dict['definitions']:
                        print 'From %s, %s:' % (d['db'], d['db_fullname'].
                                                encode ('utf_8'))
                        print d['desc']
                elif 'error' in dict:
                    print dict['msg']
            except IndexError:
                self.__error ('No previous match')
        elif input[0] == '/':
            if len (input) > 1:
                dict = self.dc.match (self.database, self.strategy, input[1:])
                if 'matches' in dict:
                    self.last_matches = []
                    lmi = 0
                    for db in dict['matches']:
                        print 'From %s, %s:' % (db, self.__lookup_db (db).
                                                encode ('utf_8'))
                        for term in dict['matches'][db]:
                            print '%4d) "%s"' % (lmi, term.encode ('utf_8'))
                            self.last_matches.append ([db, term])
                            lmi = lmi + 1
                elif 'error' in dict:
                    print dict['msg']
            else:
                if len (self.last_matches) > 0:
                    m = {}
                    lmi = 0
                    for i, db in enumerate (self.last_matches):
                        if not db[0] in m: m[db[0]] = []
                        m[db[0]].append (self.last_matches[i][1])
                    for db in m:
                        print 'From %s, %s:' % (db, self.__lookup_db (db))
                        for term in m[db]:
                            print '%4d) "%s"' % (lmi, term)
                            lmi = lmi + 1
                else:
                    self.__error ('No previous match')
        elif input[0] == '!':
            if re.match (r'^![0-9]+$', input):
                number = int (input[1:])
                readline.insert_text (readline.get_history_item (number))
                readline.redisplay ()
        else:
            dict = self.dc.define (self.database, input)
            if 'count' in dict:
                for d in dict['definitions']:
                    print 'From %s, %s:' % (d['db'], d['db_fullname'].
                                            encode ('utf_8'))
                    print d['desc']
            elif 'error' in dict:
                print dict['msg']

    def parse_command (self, input):
        input = input.split (' ', 1)
        cmd = input[0]
        args = None
        if len (input) == 2:
            args = input[1]

        if cmd == 'open':
            try:
                if args != None:
                    args = args.split (' ', 1)
                    if len (args) == 2:
                        self.dc.open (args[0], int (args[1]))
                    else:
                        self.dc.open (args[0])
                else:
                    self.dc.open ()
                dict = self.dc.show_databases ()
                self.last_databases = dict['databases']
                dict = self.dc.show_strategies ()
                self.last_strategies = dict['strategies']
            except socket.error, (errno, strerror):
                self.__error (strerror)
        elif cmd == 'close':
            self.dc.close ()
        elif cmd == 'database':
            if args != None:
                self.database = args
            else:
                print self.database
        elif cmd == 'strategy':
            if args != None:
                self.strategy = args
            else:
                print self.strategy
        elif cmd == 'distance':
            if args != None:
                self.dc.levenshtein_distance = int (args)
            else:
                if self.dc.levenshtein_distance:
                    print 'Configured Levenshtein distance: %u' % \
                        self.dc.levenshtein_distance
                else:
                    print 'No distance configured'
        elif cmd == 'ls':
            dict = self.dc.show_strategies ()
            self.last_strategies = dict['strategies']
            if len (self.last_strategies):
                for i in self.last_strategies:
                    print '%s "%s"' % (i[0], i[1])
        elif cmd == 'ld':
            dict = self.dc.show_databases ()
            self.last_databases = dict['databases']
            if len (self.last_databases):
                for i in self.last_databases:
                    print '%s "%s"' % (i[0], i[1])
        elif cmd == 'mime':
            print self.dc.option ('MIME')
        elif cmd == 'server':
            dict = self.dc.show_server ()
            if 'desc' in dict:
                print dict['desc']
            elif 'error' in dict:
                self.__error (dict['error'] + ' ' + dict['msg'])
        elif cmd == 'info':
            if args != None:
                dict = self.dc.show_info (args)
                if 'desc' in dict:
                    print dict['desc']
                elif 'error' in dict:
                    self.__error (dict['error'] + ' ' + dict['msg'])
        elif cmd == 'history':
            hl = int (readline.get_current_history_length ())
            for i in xrange (0, hl):
                print '%4d) %s' % (i, readline.get_history_item (i))
        elif cmd == 'help':
            self.print_help ()
        elif cmd == 'transcript':
            if args != None:
                if args in ('yes', 'on', 'true'):
                    self.dc.transcript = True
                elif args in ('no', 'off', 'false'):
                    self.dc.transcript = False
                else:
                    self.__error ('Expected boolean value')
            else:
                if self.dc.transcript:
                    print 'transcript is on'
                else:
                    print 'transcript is off'
        elif cmd == 'verbose':
            if args != None:
                self.dc.verbose = args
            else:
                print self.dc.verbose
        elif cmd == 'prompt':
            if args != None:
                self.prompt = args
            else:
                self.__error ('not enough arguments')
        elif cmd == 'prefix':
            if args != None:
                if len (args) == 1 and args != '#' and \
                        curses.ascii.ispunct (args):
                    self.prefix = args
                else:
                    self.__error ('Expected a single punctuation character')
        elif cmd == 'version':
            self.print_version ()
        elif cmd == 'warranty':
            self.print_warranty ()
        elif cmd == 'quit':
            sys.exit ()

    def __lookup_db (self, db):
        for d in self.last_databases:
            if d[0] == db:
                return d[1]
        return ''

    def __error (self, msg):
        print 'dico: Error: %s' % msg

    def print_version (self):
        print 'GNU Dico (Python Edition) ' + dicoclient.__version__

    def print_warranty (self):
        self.print_version ()
        print """Copyright (C) 2008, 2009, 2010 Wojciech Polak

   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>."""

    def print_help (self):
        print 'WORD                     Define WORD.'
        print '/WORD                    Match WORD.'
        print '/                        Redisplay previous matches.'
        print 'NUMBER                   Define NUMBERth match.'
        print '!NUMBER                  Edit NUMBERth previous command.'
        print
        print self.prefix + 'open [HOST [PORT]]      Connect to a DICT server.'
        print self.prefix + 'close                   Close the connection.'
        print self.prefix + 'database [NAME]         Set or display current database name.'
        print self.prefix + 'strategy [NAME]         Set or display current strategy.'
        print self.prefix + 'distance [NUM]          Set or query Levenshtein distance (server-dependent).'
        print self.prefix + 'ls                      List available matching strategies'
        print self.prefix + 'ld                      List all accessible databases'
        print self.prefix + 'info [DB]               Display the information about the database.'
        print self.prefix + 'prefix [CHAR]           Set or display command prefix.'
        print self.prefix + 'transcript [BOOL]       Set or display session transcript mode.'
        print self.prefix + 'verbose [NUMBER]        Set or display verbosity level.'
        print self.prefix + 'prompt STRING           Change command line prompt.'
        print self.prefix + 'history                 Display command history.'
        print self.prefix + 'help                    Display this help text.'
        print self.prefix + 'version                 Print program version.'
        print self.prefix + 'warranty                Print copyright statement.'
        print self.prefix + 'quit                    Quit the shell.'

if __name__ == '__main__':
    try:
        opts, args = getopt.getopt (sys.argv[1:], 'h:', ['host='])
    except getopt.GetoptError:
        print '\nusage: %s [-h, --host=hostname]' % (sys.argv[0])
        sys.exit (0)

    shell = Shell (opts, args)
    shell.print_version ()
    shell.run ()
