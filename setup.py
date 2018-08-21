#!/usr/bin/env python

from os import path
from io import open
try:
    from skbuild import setup
except ImportError:
    from setuptools import setup

here = path.abspath(path.dirname(__file__))
with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name             = 'graphite2',
    version          = '1.3.11',
    description      = 'SIL graphite2 smart font system python bindings',
    author           = 'SIL International',
    license          = 'LGPL-2.1+ OR MPL-1.1+',
    url              = 'https://github.com/silnrsi/graphite',
    zip_safe         = False,
    py_modules       = ['graphite2'],
    install_requires = ['future'],
    long_description = long_description,
    long_description_content_type = 'text/markdown',
    classifiers = [
        'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
        'License :: OSI Approved :: Mozilla Public License 1.1 (MPL 1.1)',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3'
    ]
)
