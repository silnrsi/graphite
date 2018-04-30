#!/usr/bin/env python3

import argparse, codecs, struct, io, os.path, operator
from functools import partial

def map_common(args):
    return (struct.pack("<3L2HB", args.script, args.lang,
                        tag4cc(args.feat[0]), args.feat[1], 0x409, _encodings[args.encoding]),
            [os.path.splitext(os.path.basename(args.font.name))[0],
             args.feat[0] and '{0[0]!s}={0[1]:d}'.format(args.feat),
             args.encoding])

def map_segment(args):
    ps,ns = map_common(args)
    text = args.text.read().encode(args.encoding)
    return (ps + struct.pack("<B128s", _bidi[args.direction], text),
            ns + [os.path.splitext(os.path.basename(args.text.name))[0], args.direction])

_formats = {
    'font'      : map_common,
    'segment'   : map_segment,
}

_encodings =  {
    'utf8'  : 1,
    'utf16' : 2,
    'utf32' : 4
}

_bidi = { 'ltr' : 0, 'rtl':1 }

def feature(arg: str):
    k,v = [x.strip() for x in arg.split('=')]
    return (k, int(v,0))

def tag4cc(arg: str):
    return struct.unpack_from('>L', bytes(arg, 'ascii') + b'\0'*4)[0]

parser = argparse.ArgumentParser(description='Generate fuzzer seed file')
formatparsers = parser.add_subparsers(metavar='FUZZER-TARGET',
                            dest='format_spec',
                            help='Which fuzzer to generate a seed file for.')
font_parser = formatparsers.add_parser('font', help='font API fuzzer')

seg_parser = formatparsers.add_parser('segment', help='segment API fuzzer')
seg_parser.add_argument('text', metavar='TEXT-FILE',
                        type=argparse.FileType('r',encoding='utf-8'),
                        help='Text file to fill test string from')
seg_parser.add_argument('direction', metavar='DIR', nargs='?',
                        choices=_bidi, default='ltr',
                        help='Text direction')

parser.add_argument('font', metavar='FONT', type=argparse.FileType('rb'),
                    help='Graphite enabled font file.')
parser.add_argument('lang', metavar='LANGID', type=tag4cc,
                    help='Language Id')
parser.add_argument('script', metavar='SCRIPTID', type=tag4cc,
                    help='Script Id')
parser.add_argument('encoding', metavar='UTF-FORM',
                    choices=_encodings,
                    help='UTF encoding form')
parser.add_argument('--feat', type=feature, default=('',0xffff),
                    help='A feature to set')

args = parser.parse_args()

params,names = _formats[args.format_spec](args)
outname = '_'.join(filter(bool,names)) + '.fuzz'

with io.open(outname, "wb") as f:
    f.write(args.font.read())
    f.write(params)
