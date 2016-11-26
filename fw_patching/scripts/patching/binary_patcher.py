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
        if (fw_pos == 0):
            print "error: patch address not in an ELF segment: 0x%X" % (self.pos)
            exit(0)
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
            
    def align(self, base, alignment):
        """
        aligns an address to a chosen alignment
        
        :param base: address to align
        :param alignment: address alignment
        """
        tmp = base - (base & (alignment-1))
        if (tmp == base):
            return base
        else:
            return tmp + alignment
            
    def check_and_create_seg(self, seg, dest_metadata):
        """
        checks if a segment falls into one at the destination ELF
        
        if the other segment is only in memory 
        (filesz < memsz, e.g. the last padding segment)
        and we are in this "virtual" region, we have to resize the
        destination segment and create a new one or even two (one for our code and one for padding)
        """
        address = seg['paddr']
        if (get_offset_in_elf(dest_metadata, address) == 0):
            for i, targetseg in enumerate(dest_metadata['segments']):
                start = targetseg['paddr']
                msize = targetseg['memsz']
    
                if (msize != 0): # not an empty segment
                    if ((address >= start) & (address < start+msize)): # our start address is in this segment
                        if (address+seg['memsz'] < start+msize): # the end address too
                            pre = 0
                            offset = targetseg['offset'] + targetseg['filesz']
                            # if we start not at the beginning of the segment we have to insert a padding segment before
                            if (self.align(start, seg['align']) != seg['paddr']): 
                                pre_seg = targetseg.copy()
                                pre_seg['memsz'] = seg['paddr'] - start
                                pre = 1
                                dest_metadata['segments'].insert(i, pre_seg)
                                dest_metadata['num_segments'] += 1
                                # correct hash table segment too! (we need more space here now)
                                dest_metadata['segments'][1]['filesz'] += dest_metadata['elf32_hdr']['phentsize']
                                
                            # insert our patch segment
                            new_seg = seg.copy()
                            offset = self.align(offset, new_seg['align'])
                            new_seg['offset'] = offset
                            offset += new_seg['filesz']
                            dest_metadata['segments'].insert(i + pre, new_seg)
                                
                            # if there is free space after the segment we have to insert a padding segment after
                            # (or not to delete the old one to be precise)
                            if (targetseg['memsz'] > seg['memsz']):
                                post_start = new_seg['paddr'] + new_seg['memsz']
                                end_adr = targetseg['paddr'] + targetseg['memsz']
                                offset = self.align(offset, targetseg['align'])
                                targetseg['offset'] = offset
                                targetseg['vaddr'] = new_seg['vaddr'] + new_seg['memsz']
                                targetseg['paddr'] = new_seg['paddr'] + new_seg['memsz']
                                targetseg['memsz'] = end_adr - targetseg['paddr']
                                dest_metadata['num_segments'] += 1
                                # correct hash table segment too! (we need more space here now)
                                dest_metadata['segments'][1]['filesz'] += dest_metadata['elf32_hdr']['phentsize']
                            else:
                                dest_metadata['segments'].remove(targetseg)
                            return dest_metadata
        return dest_metadata
            
    
    def apply(self, firmware, metadata, **kargs):
        for i, seg in enumerate(self.metadata['segments']):
            # segment type 1 is LOAD
            if (seg['type'] == 1):
                start_adr = seg['paddr']
                if (seg['memsz'] != 0):
                    self.pos = start_adr
                    metadata = self.check_and_create_seg(seg, metadata)
                    self.data = self.get_bytes(seg)
                    if (seg['filesz']):
                        firmware = super(ElfPatch, self).apply(firmware, metadata)
        firmware = update_metadata(firmware, metadata)
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

        
def patch_firmware(src, dst, patches, **kargs):
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
    firmware = file(dst,'wb').write(firmware)
    print "firmware patching finished"

