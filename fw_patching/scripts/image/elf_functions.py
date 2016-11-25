import struct
 
def gen_struct(format, image):
  """Generates a dictionary from the format tuple by reading image"""

  str = "<%s" % "".join([x[1] for x in format])
  elems = struct.unpack(str, image.read(struct.calcsize(str)))
  keys = [x[0] for x in format]
  return dict(zip(keys, elems))

def parse_metadata(image):
  """Parses elf header metadata"""
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

def is_elf(file):
  """Verifies a file as being an ELF file"""
  file.seek(0)
  magic = file.read(4)
  file.seek(0)
  return magic == '\x7fELF'
