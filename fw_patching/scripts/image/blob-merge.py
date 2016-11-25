#!/usr/bin/env python2

"""
file: blob_merge.py

Merges blobs (segments) into a single elf file

This file is largely based on the pil-splitter.py script from Qualcomm and:
https://github.com/remittor/qcom-mbn-tools

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
from elf_functions import *

def die(message):
  print message
  exit(1)

def usage():
  print "Usage: %s <prefix> <elf>" % sys.argv[0]
  exit(1)

def write_segments(metadata, prefix, output):
  """
  writes all blobs into a single file, with the correct offset specified in the ELF header
  
  :param metadata: parsed metadata
  :param prefix: prefix of input files, usually "modem"
  :param output: destination file name
  """
  outFile = open(output, 'wb')
    
  for i, seg in enumerate(metadata['segments']):
    start = seg['offset']
    size = seg['filesz']
    
    if (size != 0):
    
      inputName = "%s.b%02d" % (prefix, i)
    
      inFile = open(inputName, 'rb')

      outFile.seek(start)
      outFile.write(inFile.read(size))
  outFile.close()  

if __name__ == "__main__":
  """
  main function: read in metadata, merge blob files
  """
  if len(sys.argv) != 3:
    usage()
 
  prefix = sys.argv[1]

  b00name = "%s.b00" % (prefix)
  b00 = open(b00name, 'rb')
  if not is_elf(b00):
    usage()

  metadata = parse_metadata(b00)
  write_segments(metadata, prefix, sys.argv[2])
 
