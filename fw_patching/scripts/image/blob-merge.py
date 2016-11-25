#! /usr/bin/env python
"""
Merges blobs (segments) into a single elf file (.mbn)

This file is largely based on the pil-splitter.py script
"""

import sys
from elf_functions import *

def die(message):
  print message
  exit(1)

def usage():
  print "Usage: %s <prefix> <elf>" % sys.argv[0]
  exit(1)

# writes all blobs into a single file, with the correct offset specified in the ELF header
def write_segments(metadata, prefix, output):
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

  if len(sys.argv) != 3:
    usage()
 
  prefix = sys.argv[1]

  b00name = "%s.b00" % (prefix)
  b00 = open(b00name, 'rb')
  if not is_elf(b00):
    usage()

  metadata = parse_metadata(b00)
  write_segments(metadata, prefix, sys.argv[2])
 
