#!/usr/bin/env python
import sys

LICENSE = '''
//%LICENSE////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Devchandra M. Leishangthem (dlmeetei at gmail dot com)
//
// Distributed under the MIT License (See accompanying file LICENSE)
//
//////////////////////////////////////////////////////////////////////////
//
//%///////////////////////////////////////////////////////////////////////////
'''

if len(sys.argv) < 2:
    sys.stderr.write('usage: ' + sys.argv[0] + ' file\n' )
    sys.exit(1)
#if len

with file(sys.argv[1], 'r') as original: data = original.read()
with file(sys.argv[1], 'w') as modified: modified.write(LICENSE + '\n' + data)
