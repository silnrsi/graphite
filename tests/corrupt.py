#!/usr/bin/python
import optparse, os, shutil, sys

def revert(path):
    bkup = path + os.path.extsep + options.backup_suffix
    if os.access(bkup, os.R_OK):
        shutil.copy2(bkup, path)
        os.remove(bkup)


def corrupt(path, offset, value):
    if options.backup:
        shutil.copy2(path, path + os.path.extsep + options.backup_suffix)
    with open(path,'r+b',buffering=0) as f:
        f.seek(offset)
        f.write(bytes(chr(value)))


parser = optparse.OptionParser(usage='usage: %prog file byte-offset replacment-value')
parser.add_option("", "--revert", action="store_true", default=False,
        help="restore the path to pristine condition if possible.")
parser.add_option("-b", "--backup", action="store_true", default=True, dest="backup",
        help="create a backup of the uncorrupted original [default: %default]")
parser.add_option("", "--no-backup", action="store_false", dest="backup",
        help="do not create a backup of the uncorrupted original.")
parser.add_option("", "--backup-suffix", type="string", default="pristine",
        help="suffix for uncorrupted copy of the file [default: %default]")
(options, args) = parser.parse_args()

if options.revert:
    if len(args) != 1:
        parser.print_help()
        sys.exit(1)
elif len(args) != 3:
    parser.print_help()
    sys.exit(1)

path   = args[0]
revert(path)

if not options.revert:
    offset = int(eval(args[1]))
    value  = int(eval(args[2]))
    corrupt(path, offset, value)

