#! /usr/bin/python3
# vim: set sw=2 ts=8 sts=2 et:

#from __future__ import print_function
import argparse
import sys
import os
import re

def eprint(*args, **kwargs):
  print(*args, file=sys.stderr, **kwargs)

def main():
  PROG=os.path.basename(sys.argv[0])

  help='''{prog} removes difference between golden and test VCDs caused by different SystemC versions.'''.format(prog=PROG)

  parser = argparse.ArgumentParser(description=help, prog=PROG)
  parser.add_argument("--golden", type=str, help="specifies golden VCD file", required=True)
  parser.add_argument("--test", type=str, help="specifies test VCD file to be adjusted", required=True)
  args = parser.parse_args()

  try:
    GOLDEN = open(args.golden, 'r')
  except (IOError, OSError) as e:
    eprint("Can't open golden VCD file '{}':".format(args.golden), e)
    return -1

  reTimeStamp = re.compile('^#([0-9]+)$')

  lastTimeStamp = 0

  for line in GOLDEN:
    m = reTimeStamp.match(line)
    if m:
      lastTimeStamp = int(m.group(1))

  try:
    TEST = open(args.test, 'r')
  except (IOError, OSError) as e:
    eprint("Can't open test VCD file '{}':".format(args.test), e)
    return -1

  try:
    OUT = open(args.test+".tmp", 'w')
  except (IOError, OSError) as e:
    eprint("Can't open temporary VCD file '{}':".format(args.test+".tmp"), e)
    return -1

  outbuf = ""

  for line in TEST:
    m = reTimeStamp.match(line)
    if m:
      if int(m.group(1)) <= lastTimeStamp:
        OUT.write(outbuf)
      else:
        break
      outbuf = ""
    outbuf += line

  OUT.write(outbuf)

  try:
    os.rename(args.test+".tmp", args.test)
  except (IOError, OSError) as e:
    eprint("Can't rename temporary VCD file to '{}':".format(args.test), e)
    return -1

  return 0

if __name__ == '__main__':
  exit(main())
