#!/usr/bin/env python

from skbuild import setup
from os import path
from io import open

here = path.abspath(path.dirname(__file__))
with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name             = 'graphite2',
    version          = '1.3.11',
    description      = 'SIL graphite2 smart font system python bindings',
    author           = 'SIL International',
    license          = 'LGPL-2.1',
    url              = 'https://github.com/silnrsi/graphite',
    zip_safe         = False,
    py_modules       = ['graphite2'],
    install_requires = ['future'],
    long_description = long_description,
    long_description_content_type = 'text/markdown'
)
