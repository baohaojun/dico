#  Django settings for Dicoweb project.
#
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

import os
SITE_ROOT = os.path.dirname (os.path.realpath (__file__))

DEBUG = True
TEMPLATE_DEBUG = True

ADMINS = (
    ('Your Name', 'Your e-mail address'),
)
MANAGERS = ADMINS

DATABASE_ENGINE = ''
DATABASE_NAME = ''
DATABASE_USER = ''
DATABASE_PASSWORD = ''
DATABASE_HOST = ''
DATABASE_PORT = ''

SITE_ID = 1
USE_I18N = True

TIME_ZONE = 'Europe/Warsaw'
LANGUAGE_CODE = 'en-us'
LANGUAGE_COOKIE_NAME = 'dicoweb_lang'

SESSION_COOKIE_NAME = 'dicoweb_sid'
SESSION_ENGINE = 'django.contrib.sessions.backends.file'
SESSION_EXPIRE_AT_BROWSER_CLOSE = True

# Caching, see http://docs.djangoproject.com/en/dev/topics/cache/#topics-cache
CACHE_BACKEND = 'memcached://127.0.0.1:11211/'

# Absolute path to the directory that holds media/static files.
MEDIA_ROOT = os.path.join (SITE_ROOT, 'static')

# URL that handles the media served from MEDIA_ROOT.
MEDIA_URL = 'static'

# Make this unique, and don't share it with anybody.
SECRET_KEY = 'SET THIS TO A RANDOM STRING'

# List of callables that know how to import templates from various sources.
TEMPLATE_LOADERS = (
    'django.template.loaders.filesystem.load_template_source',
    'django.template.loaders.app_directories.load_template_source',
)

MIDDLEWARE_CLASSES = (
    'django.middleware.cache.UpdateCacheMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.locale.LocaleMiddleware',
    'django.middleware.gzip.GZipMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.cache.FetchFromCacheMiddleware',
)

ROOT_URLCONF = 'dicoweb.urls'

TEMPLATE_DIRS = (
    os.path.join (SITE_ROOT, 'templates'),
)

INSTALLED_APPS = (
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.sites',
    'dicoweb',
)

DICT_SERVERS = ('gnu.org.ua',)
DICT_TIMEOUT = 10
