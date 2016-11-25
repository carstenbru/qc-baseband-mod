#!/usr/bin/env python2

"""
file: pil-splitter.py

Splits ELF files into segments.

If the ELF is signed, the ELF headers and the hash segment are output to
the *.mdt file, and then the segments are output to *.b<n> files.

If the ELF isn't signed each segment is output to a *.b<n> file and the
ELF headers are output to the *.mdt file.

This file is largely based on the pil-splitter.py script from Qualcomm and:
https://github.com/remittor/qcom-mbn-tools

modified by Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
import struct
from elf_functions import *

def die(message):
  print message
  exit(1)

def usage():
  print "Usage: %s <elf> <prefix>" % sys.argv[0]
  exit(1)

def dump_data(input, output, start, size):
  """
  Dump 'size' bytes from 'input' at 'start' into newfile 'output'
  """

  if size == 0:
    return

  input.seek(start)
  outFile = open(output, 'wb')
  outFile.write(input.read(size))
  outFile.close()

  #print 'BIN %s' % output

def append_data(input, output, start, size):
  """
  Append 'size' bytes from 'input' at 'start' to 'output' file
  """

  if size == 0:
    return

  input.seek(start)
  outFile = open(output, 'ab')
  outFile.write(input.read(size))
  outFile.close()


def dump_metadata(metadata, image, name):
  """
  Creates <name>.mdt file from elf metadata
  """

  name = "%s.mdt" % name
  # Dump out the elf header
  dump_data(image, name, 0, 52)
  # Append the program headers
  append_data(image, name, metadata['pg_start'], 32 * metadata['num_segments'])

  for seg in metadata['segments']:
    if seg['hash']:
      append_data(image, name, seg['offset'], seg['filesz'])
      break

def dump_segments(metadata, image, name):
  """
  Creates <name>.bXX files for each segment
  """
  for i, seg in enumerate(metadata['segments']):
    start = seg['offset']
    size = seg['filesz']
    output = "%s.b%02d" % (name, i)
    dump_data(image, output, start, size)

if __name__ == "__main__":

  if len(sys.argv) != 3:
    usage()

  image = open(sys.argv[1], 'rb')
  if not is_elf(image):
    usage()

  prefix = sys.argv[2]
  metadata = parse_metadata(image)
  dump_metadata(metadata, image, prefix)
  dump_segments(metadata, image, prefix)
