#!/usr/bin/env python2

"""
file: elf_functions.py

ELF file processing functions

This file is largely based on the pil-splitter.py script from Qualcomm and:
https://github.com/remittor/qcom-mbn-tools

modified by Carsten Bruns (carst.bruns@gmx.de)
"""

import struct

def get_offset_in_elf(metadata, address):
    """
    gets the offset in the target ELF file (file position)
    of a memory address and the number segment
    
    :param metadata: parsed metdata information of the ELF file
    :param address: address to resolve
    """
    for i, seg in enumerate(metadata['segments']):
        start = seg['paddr']
        size = seg['filesz']
    
        if (size != 0):
            if ((address >= start) & (address < start+size)):
                return (seg['offset'] + address - start), i
    return 0, 0
 
def gen_struct(format, image):
  """
  Generates a dictionary from the format tuple by reading image
  
  :param format: struct format
  :param image: input data
  """

  str = "<%s" % "".join([x[1] for x in format])
  elems = struct.unpack(str, image.read(struct.calcsize(str)))
  keys = [x[0] for x in format]
  return dict(zip(keys, elems))

def parse_metadata(image):
  """
  Parses elf header metadata
  
  :param image: input ELF file
  """
  metadata = {}

  elf32_hdr = [
      ("ident", "16s"),
      ("type", "H"),
      ("machine", "H"),
      ("version", "I"),
      ("entry", "I"),
      ("phoff", "I"),
      ("shoff", "I"),
      ("flags", "I"),
      ("ehsize", "H"),
      ("phentsize", "H"),
      ("phnum", "H"),
      ("shentsize", "H"),
      ("shnum", "H"),
      ("shstrndx", "H"),
      ]
  elf32_hdr = gen_struct(elf32_hdr, image)
  metadata['elf32_hdr'] = elf32_hdr
  metadata['num_segments'] = elf32_hdr['phnum']
  metadata['pg_start'] = elf32_hdr['phoff']

  elf32_phdr = [
      ("type", "I"),
      ("offset", "I"),
      ("vaddr", "I"),
      ("paddr", "I"),
      ("filesz", "I"),
      ("memsz", "I"),
      ("flags", "I"),
      ("align", "I"),
      ]

  #print "pg_start = 0x%08x" % metadata['pg_start']

  metadata['segments'] = []  
  for i in xrange(metadata['num_segments']):
    poff = metadata['pg_start'] + (i * elf32_hdr['phentsize'])
    image.seek(poff)
    phdr = gen_struct(elf32_phdr, image)
    metadata['segments'].append(phdr)
    phdr['hash'] = (phdr['flags'] & (0x7 << 24)) == (0x2 << 24)
    #print "["+str(i)+"] %08x" % poff," offset =",phdr['offset']," size =",phdr['filesz']

  return metadata

def write_struct(format, structi):
    data = ""
    for x in format:
        data += struct.pack("<%s" % x[1], structi[x[0]])
    return data

def update_metadata(firmware, metadata):
    elf32_hdr = [ #TODO only once in file
      ("ident", "16s"),
      ("type", "H"),
      ("machine", "H"),
      ("version", "I"),
      ("entry", "I"),
      ("phoff", "I"),
      ("shoff", "I"),
      ("flags", "I"),
      ("ehsize", "H"),
      ("phentsize", "H"),
      ("phnum", "H"),
      ("shentsize", "H"),
      ("shnum", "H"),
      ("shstrndx", "H"),
      ]
    
    metadata['elf32_hdr']['phnum'] = metadata['num_segments']
    elf_data = write_struct(elf32_hdr, metadata['elf32_hdr'])
    #num_segments = struct.pack("<H", metadata['num_segments'])
    firmware = elf_data + firmware[len(elf_data):]
    
    #TODO update elf32_hdr (num_segments)
    elf32_phdr = [
      ("type", "I"),
      ("offset", "I"),
      ("vaddr", "I"),
      ("paddr", "I"),
      ("filesz", "I"),
      ("memsz", "I"),
      ("flags", "I"),
      ("align", "I"),
      ]
    
    for i, seg in enumerate(metadata['segments']):
        poff = metadata['pg_start'] + (i * metadata['elf32_hdr']['phentsize'])
        data = write_struct(elf32_phdr, seg)
        firmware = firmware[:poff] + data + firmware[poff+len(data):]
    return firmware

def is_elf(file):
  """
  Verifies a file as being an ELF file
  
  :param file: input file to test
  """
  file.seek(0)
  magic = file.read(4)
  file.seek(0)
  return magic == '\x7fELF'
