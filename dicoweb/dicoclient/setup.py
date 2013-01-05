#!/usr/bin/env python
#
#  This file is part of GNU Dico.
#  Copyright (C) 2008-2009, 2012 Wojciech Polak
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

from distutils.core import setup
import dicoclient

setup(name='dicoclient',
      version='1.0',
      author='Wojciech Polak',
      author_email='polak@gnu.org',
      url='http://www.gnu.org/software/dico/',
      py_modules=['dicoclient', 'dicoshell'],
      license='GPL License',
      description='A DICT protocol (RFC 2229) client library.',
      platforms=['any'],
      classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules'
      ]
)
