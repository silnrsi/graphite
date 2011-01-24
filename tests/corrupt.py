#!/usr/bin/env python3.1
import sys
import optparse

parser = optparse.OptionParser(usage='usage: %prog file byte-offset replacment-value')
(options, args) = parser.parse_args()

if len(args) != 3:
    parser.print_help()
    sys.exit(1)

path   = args[0]
offset = eval(args[1])
value  = eval(args[2])

with open(path,'r+b',buffering=0) as f:
    f.seek(offset)
    f.write(bytes([value]))

