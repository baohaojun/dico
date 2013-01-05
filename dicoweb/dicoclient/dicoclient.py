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
import socket
import base64
import quopri

__version__ = '1.0'

class DicoClient:
    """GNU Dico client module written in Python
       (a part of GNU Dico software)"""

    host = None;
    levenshtein_distance = 0
    mime = False

    verbose = 0
    timeout = 10
    transcript = False
    __connected = False

    def __init__ (self, host=None):
        if host != None:
            self.host = host;

    def __del__ (self):
        if self.__connected:
            self.socket.close ()

    def open (self, host=None, port=2628):
        """Open the connection to the DICT server."""
        if host != None:
            self.host = host
        if self.verbose:
            self.__debug ('Connecting to %s:%d' % (self.host, port))
        socket.setdefaulttimeout (int (self.timeout))
        self.socket = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.connect ((self.host, port))
        self.__connected = True
        self.fd = self.socket.makefile ();

        self.server_banner = self.__read ()[0]
        capas, msgid = re.search ('<(.*)> (<.*>)$',
                                  self.server_banner).groups ()
        self.server_capas = capas.split ('.')
        self.server_msgid = msgid

        self.__send_client ()
        self.__read ()

    def close (self):
        """Close the connection."""
        if self.__connected:
            self.__send_quit ()
            self.__read ()
            self.socket.close ()
            self.__connected = False

    def option (self, name, *args):
        """Send the OPTION command."""
        if self.__connected:
            self.__send ('OPTION %s%s' %
                         (name, reduce (lambda x, y: str (x) +' '+ str (y),
                                        args, '')))
            res = self.__read ()
            code, msg = res[0].split (' ', 1)
            if int (code) == 250:
                if name.lower () == 'mime':
                    self.mime = True
                return True
            return False

    def __get_mime (self, lines):
        cnt = 0
        mimeinfo = {}
        firstline = lines[0].lower ()
        if firstline.find ('content-type:') != -1 or \
           firstline.find ('content-transfer-encoding:') != -1:
            cnt += 1
            for line in lines:
                if line == '':
                    break
                t = line.split (':', 1)
                mimeinfo[t[0].lower ()] = t[1].strip ()
                cnt += 1
            for i in range (0, cnt):
                lines.pop (0)
        else:
            lines.pop (0)
        if 'content-transfer-encoding' in mimeinfo:
            if mimeinfo['content-transfer-encoding'].lower () == 'base64':
                buf = base64.decodestring ('\n'.join (lines))
                lines[:] = (buf.split ('\r\n'))
                if lines[-1] == '': del lines[-1]
                del mimeinfo['content-transfer-encoding']
            elif mimeinfo['content-transfer-encoding'].lower () == 'quoted-printable':
                buf = quopri.decodestring ('\n'.join (lines))
                lines[:] = buf.split ('\r\n')
                if lines[-1] == '': del lines[-1]
                del mimeinfo['content-transfer-encoding']
        return mimeinfo

    def __get_rs (self, line):
        code, text = line.split (' ', 1)
        code = int (code)
        return code, text

    def __read (self):
        if not self.__connected:
            raise DicoNotConnectedError ('Not connected')
        buf = []
        line = self.__readline ()
        if len (line) == 0:
            raise DicoNotConnectedError ('Not connected')
        buf.append (line)
        code, text = self.__get_rs (line)

        if code >= 100 and code < 200:
            if code == 150:
                while True:
                    rs = self.__readline ()
                    code, text = self.__get_rs (rs)
                    if code != 151:
                        buf.append (rs)
                        break
                    buf.append ([rs, self.__readblock ()])
            else:
                buf.append (self.__readblock ())
                buf.append (self.__readline ())
        return buf

    def __readline (self):
        line = self.fd.readline ().rstrip ()
        if self.transcript:
            self.__debug ('S:%s' % line)
        return line

    def __readblock (self):
        buf = []
        while True:
            line = self.__readline ()
            if line == '.':
                break
            buf.append (line)
        return buf


    def __send (self, command):
        if not self.__connected:
            raise DicoNotConnectedError ('Not connected')
        self.socket.send (command.encode ('utf_8') + "\r\n")
        if self.transcript:
            self.__debug ('C:%s' % command)

    def __send_client (self):
        if self.verbose:
            self.__debug ('Sending client information')
        self.__send ('CLIENT "%s %s"' % ("GNU Dico (Python Edition)",
                                         __version__))

    def __send_quit (self):
        if self.verbose:
            self.__debug ('Quitting')
        self.__send ('QUIT');

    def __send_show (self, what, arg=None):
        if arg != None:
            self.__send ('SHOW %s "%s"' % (what, arg))
        else:
            self.__send ('SHOW %s' % what)
        return self.__read ()

    def __send_define (self, database, word):
        if self.verbose:
            self.__debug ('Sending query for word "%s" in database "%s"' %
                          (word, database))
        self.__send ('DEFINE "%s" "%s"' % (database, word))
        return self.__read ()

    def __send_match (self, database, strategy, word):
        if self.verbose:
            self.__debug ('Sending query to match word "%s" in database "%s", using "%s"'
                          % (word, database, strategy))
        self.__send ('MATCH "%s" "%s" "%s"' % (database, strategy, word))
        return self.__read ()

    def __send_xlev (self, distance):
        self.__send ('XLEV %u' % distance)
        return self.__read ()

    def show_databases (self):
        """List all accessible databases."""
        if self.verbose:
            self.__debug ('Getting list of databases')
        res = self.__send_show ('DATABASES')
        if self.mime:
            mimeinfo = self.__get_mime (res[1])
        dbs_res = res[1:-1][0]
        dbs = []
        for d in dbs_res:
            short_name, full_name = d.split (' ', 1)
            dbs.append ([short_name, self.__unquote (full_name)])
        dct = {
            'count': len (dbs),
            'databases': dbs,
        }
        return dct

    def show_strategies (self):
        """List available matching strategies."""
        if self.verbose:
            self.__debug ('Getting list of strategies')
        res = self.__send_show ('STRATEGIES')
        if self.mime:
            mimeinfo = self.__get_mime (res[1])
        sts_res = res[1:-1][0]
        sts = []
        for s in sts_res:
            short_name, full_name = s.split (' ', 1)
            sts.append ([short_name, self.__unquote (full_name)])
        dct = {
            'count': len (sts),
            'strategies': sts,
        }
        return dct

    def show_info (self, database):
        """Provide information about the database."""
        res = self.__send_show ("INFO", database)
        code, msg = res[0].split (' ', 1)
        if int (code) < 500:
            if self.mime:
                mimeinfo = self.__get_mime (res[1])
            dsc = res[1]
            return {'desc': '\n'.join (dsc)}
        else:
            return {'error': code, 'msg': msg}

    def show_lang_db (self):
        """Show databases with their language preferences."""
        res = self.__send_show ('LANG DB')
        code, msg = res[0].split (' ', 1)
        if int (code) < 500:
            if self.mime:
                mimeinfo = self.__get_mime (res[1])
            dsc = res[1]
            lang_src = {}
            lang_dst = {}
            for i in dsc:
                pair = i.split (' ', 1)[1]
                src, dst = pair.split (':', 1)
                for j in src:
                    lang_src[src.strip()] = True
                for j in dst:
                    lang_dst[dst.strip()] = True
            return {
                'desc': '\n'.join (dsc),
                'lang_src': lang_src.keys (),
                'lang_dst': lang_dst.keys (),
            }
        else:
            return {'error': code, 'msg': msg}

    def show_lang_pref (self):
        """Show server language preferences."""
        res = self.__send_show ('LANG PREF')
        code, msg = res[0].split (' ', 1)
        if int (code) < 500:
            return {'msg': msg}
        else:
            return {'error': code, 'msg': msg}

    def show_server (self):
        """Provide site-specific information."""
        res = self.__send_show ('SERVER')
        code, msg = res[0].split (' ', 1)
        if int (code) < 500:
            dsc = res[1]
            return {'desc': '\n'.join (dsc)}
        else:
            return {'error': code, 'msg': msg}

    def define (self, database, word):
        """Look up word in database."""
        database = database.replace ('"', "\\\"")
        word = word.replace ('"', "\\\"")
        res = self.__send_define (database, word)
        code, msg = res[-1].split (' ', 1)
        if int (code) < 500:
            defs_res = res[1:-1]
            defs = []
            rx = re.compile ('^\d+ ("[^"]+"|\w+) ([a-zA-Z0-9_\-]+) ("[^"]*"|\w+)')
            for i in defs_res:
                term, db, db_fullname = rx.search (i[0]).groups ()
                df = {
                    'term': self.__unquote (term),
                    'db': db,
                    'db_fullname': self.__unquote (db_fullname),
                }
                if self.mime:
                    mimeinfo = self.__get_mime (i[1])
                    df.update (mimeinfo)
                df['desc'] = '\n'.join (i[1])
                defs.append (df)
            dct = {
                'count': len (defs),
                'definitions': defs,
            }
            return dct
        else:
            return {'error': code, 'msg': msg}

    def match (self, database, strategy, word):
        """Match word in database using strategy."""
        if not self.__connected:
            raise DicoNotConnectedError ('Not connected')

        if self.levenshtein_distance and 'xlev' in self.server_capas:
            res = self.__send_xlev (self.levenshtein_distance)
            code, msg = res[-1].split (' ', 1)
            if int (code) != 250 and self.verbose:
                self.__debug ('Server rejected XLEV command')
                self.__debug ('Server reply: %s' % msg)

        database = database.replace ('"', "\\\"")
        strategy = strategy.replace ('"', "\\\"")
        word = word.replace ('"', "\\\"")

        res = self.__send_match (database, strategy, word)
        code, msg = res[-1].split (' ', 1)
        if int (code) < 500:
            if self.mime:
                mimeinfo = self.__get_mime (res[1])
            mts_refs = res[1:-1][0]
            mts = {}
            for i in mts_refs:
                db, term = i.split (' ', 1)
                if mts.has_key (db):
                    mts[db].append (self.__unquote (term))
                else:
                    mts[db] = [self.__unquote (term)]
            dct = {
                'matches': mts,
            }
            return dct
        else:
            return {'error': code, 'msg': msg}

    def xlev (self, distance):
        """Set Levenshtein distance."""
        self.levenshtein_distance = distance
        res = self.__send_xlev (distance)
        code, msg = res[0].split (' ', 1)
        if int (code) == 250:
            return True
        return False

    def __unquote (self, s):
        s = s.replace ("\\\\'", "'")
        if s[0] == '"' and s[-1] == '"':
            s = s[1:-1]
        try:
            s = self.__decode (s)
        except UnicodeEncodeError:
            pass
        return s

    def __decode (self, encoded):
        for octc in (c for c in re.findall (r'\\(\d{3})', encoded)):
            encoded = encoded.replace (r'\%s' % octc, chr (int (octc, 8)))
        return unicode (encoded, 'utf_8')

    def __debug (self, msg):
        print 'dico: Debug: %s' % msg

class DicoNotConnectedError (Exception):
    def __init__ (self, value):
        self.parameter = value
    def __str__(self):
        return repr (self.parameter)
