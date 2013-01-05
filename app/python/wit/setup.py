#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2008,2009 Sergey Poznyakoff
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from distutils.core import setup
import wikimarkup

setup(name='wit',
      version='0.1',
      author='Sergey Poznyakoff',
      author_email='gray@gnu.org.ua',
      url='http://gray.gnu.org.ua/',
      package_dir = {'wit': ''},
      packages=['wit'],
      license='GPL License',
      description='Wiki markup translator.',
      platforms=['any'],
      classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Text Processing :: Markup'
      ]
)
