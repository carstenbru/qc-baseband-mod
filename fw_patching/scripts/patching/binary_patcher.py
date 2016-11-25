#!/usr/bin/env python2

"""
file: binary_patcher.py

target ELF file patching from a list of patches

Based on the 'generic_patcher' by shoote:
https://github.com/shoote/generic_patcher

modified for Qualcomm Hexagon DSP 
and the needs of this patching framework
by Carsten Bruns (carst.bruns@gmx.de)
"""

from subprocess import check_call, STDOUT 
from os import unlink
import os
from os.path import exists
import struct
import binascii

import sys
import struct

from elf_functions import *

def get_offset_in_elf(metadata, address):
    """
    gets the offset in the target ELF file (file position)
    of a memory address
    
    :param metadata: parsed metdata information of the ELF file
    :param address: address to resolve
    """
    for i, seg in enumerate(metadata['segments']):
        start = seg['paddr']
        size = seg['filesz']
    
        if (size != 0):
            if ((address >= start) & (address < start+size)):
                return seg['offset'] + address - start
    return 0

class BasePatch(object):
    """
    basis for all patches
    writes "data" to the given position
    """
    def __init__(self, pos, data):
        self.pos = pos
        self.data = data
    
    def apply(self, firmware, metadata, **kargs):
        fw_pos = get_offset_in_elf(metadata, self.pos)
        #print "patching at pos %d, length %d" % (fw_pos, len(self.data))
        firmware += max(0, fw_pos - len(firmware)) * "\x00"
        return firmware[:fw_pos] + self.data + firmware[fw_pos+len(self.data):]

class ElfPatch(BasePatch):
    """
    writes the contents of an input ELF to their corresponding
    positions in the target ELF, overwriting the original contents
    """
    def __init__(self, infile):
        elf = open(infile, 'rb')
        self.metadata = parse_metadata(elf)
        self.infile = infile

    def get_bytes(self, seg):
        try:
            offset = seg['offset']
            size = seg['filesz']
            data = file(self.infile,'rb').read()[offset:offset+size]
            return data
        except:
            print "error: could not read ELF patch file"
            
    
    def apply(self, firmware, metadata, **kargs):
        for i, seg in enumerate(self.metadata['segments']):
            # segment type 1 is LOAD
            if (seg['type'] == 1):
                start_adr = seg['paddr']
                size = seg['filesz']
                if (size != 0):
                    self.pos = start_adr
                    self.data = self.get_bytes(seg)
                    firmware = super(ElfPatch, self).apply(firmware, metadata)
        return firmware

class JumpPatch(BasePatch):
    """
    generates a jump instruction at pos to dst
    
    when the jump is too far, 
    an imext (immediate extender) is used
    """
    def __init__(self, pos, dst):
        self.pos = pos
        self.dst = dst

    def patch_gen(self, pos, dst):
        diff = (dst - pos) & 0xffffffff
        
        # check if the jump distance is small enough to encode in jump instruction
        if ((diff > 0xFFFFFF) & (diff < 0xFF800000)):
            opcode1 = 0x4000 # imext with parse bits 01 
            opcode1 |= (((diff >> 20) & 0xFFF) << 16)
            opcode1 |= ((diff >> 6) & 0x3FFF)
        
            opcode2 = 0x5800C000 # jump #r22:2
            opcode2 |= (((diff) & 0x3F) << 1)
            
            packed_data = struct.pack("<I", opcode1) + struct.pack("<I", opcode2)
        
        else:
            opcode = 0x5800C000 # jump #r22:2
            opcode |= (((diff >> 15) & 0x1FF) << 16)
            opcode |= (((diff >> 2) & 0x1FFF) << 1)
            
            packed_data = struct.pack("<I", opcode)
        
        return packed_data

    def apply(self, firmware, metadata, **kargs):
        self.data = str(self.patch_gen(self.pos, self.dst))
        return super(JumpPatch, self).apply(firmware, metadata)

class GenericPatch4(BasePatch):
    """
    replaces a 4 Byte word
    """
    def __init__(self, pos, to):
        self.pos = pos
        self.to = to

    def patch_gen(self, subst):
        packed_data = struct.pack("<I", subst)
        return packed_data

    def apply(self, firmware, metadata, **kargs):
        self.data = str(self.patch_gen(self.to))
        return super(GenericPatch4, self).apply(firmware, metadata)

class GenericPatch2(BasePatch):
    """
    replaces a 2 Byte word
    """
    def __init__(self, pos, to):
        self.pos = pos
        self.to = to

    def patch_gen(self, subst):
        packed_data = struct.pack("<H", subst)
        return packed_data

    def apply(self, firmware, metadata, **kargs):
        self.data = str(self.patch_gen(self.to))
        return super(GenericPatch2, self).apply(firmware, metadata)

class StringPatch(BasePatch):
    """
    places a string into the firmware
    """
    def __init__(self, pos, to):
        self.pos = pos
        self.to = to

    def apply(self, firmware, metadata, **kargs):
        self.data = self.to + '\0'
        return super(StringPatch, self).apply(firmware, metadata)

        
def patch_firmware(src, dst, patches, extra = "", **kargs):
    """
    patches the firmware ELF file with a list of patches

    :param src: original ELF file (input)
    :param dest: patched ELF file (output)
    :param patches: list of patches inheriting from BasePatch
    """
    elf = open(src, 'rb')
    metadata = parse_metadata(elf)
    
    firmware = file(src,'rb').read()
    for p in patches:
        firmware = p.apply(firmware, metadata, **kargs)
    firmware += extra
    firmware = file(dst,'wb').write(firmware)
    print "firmware patching finished"

