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

class BasePatch(object):
    """
    basis for all patches
    writes "data" to the given position
    """
    def __init__(self, pos, data):
        self.pos = pos
        self.data = data
    
    def apply(self, firmware, metadata, **kargs):
        fw_pos, i = get_offset_in_elf(metadata, self.pos)
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
        
    def add_segment_in_header(self, metadata):
        """
        adds a new segment in the ELF header
        
        :param metadata: destination metadata
        """
        metadata['num_segments'] += 1
        # correct hash table segment and ELF segment too! (we need more space here now)
        metadata['segments'][0]['filesz'] += metadata['elf32_hdr']['phentsize']
        metadata['segments'][1]['filesz'] += 32
        
    def get_next_offset_aligned(self, metadata, alignment):
        """
        returns the next file offset to use for a new segment
        
        :param metadata: metadata containing ELF segments
        """
        max_offset = 0
        max_pos = 0
        for i, seg in enumerate(metadata['segments']):
            offset = seg['offset']
            if (offset > max_offset):
                max_offset = offset
                max_pos = i
        pos = max_offset + metadata['segments'][max_pos]['filesz']
        return self.align(pos, alignment)
            
    def check_and_create_seg(self, seg, dest_metadata):
        """
        checks if a segment falls into one at the destination ELF
        
        if the other segment is only in memory 
        (filesz < memsz, e.g. the last padding segment)
        and we are in this "virtual" region, we have to resize the
        destination segment and create a new one or even two (one for our code and one for padding)
        """
        address = seg['paddr']
        a1, start_seg = get_offset_in_elf(dest_metadata, address)
        a2, end_seg = get_offset_in_elf(dest_metadata, address+seg['memsz'])
        
        # check if complete segment is in the same destination segment
        if (start_seg != end_seg):
            print "error: patch code ends in another segment then it starts"
            print "this can also be caused by an overflow of the destination region"
            print "start segment:%d segment:%d" % (start_seg, end_seg)
            exit(0)
            
        if (start_seg  == 0):
            last_smaller_seg = 0;
            for i, targetseg in enumerate(dest_metadata['segments']):
                start = targetseg['paddr']
                msize = targetseg['memsz']
    
                if (msize != 0): # not an empty segment
                    if (address < start+msize):
                        if ((address >= start)): # our start address is in this segment
                            if (address+seg['memsz'] < start+msize): # the end address too
                                pre = 0
                                # if we start not at the beginning of the segment we have to insert a padding segment before
                                if (self.align(start, seg['align']) != seg['paddr']): 
                                    pre_seg = targetseg.copy()
                                    pre_seg['memsz'] = seg['paddr'] - start
                                    pre = 1
                                    metadata['segments'].insert(i, pre_seg)
                                    self.add_segment_in_header(dest_metadata, pre_seg)
                                
                                # insert our patch segment
                                new_seg = seg.copy()
                                new_seg['offset'] = self.get_next_offset_aligned(dest_metadata, new_seg['align'])
                                dest_metadata['segments'].insert(i + pre, new_seg)
                                
                                # if there is free space after the segment we have to insert a padding segment after
                                # (or not to delete the old one to be precise)
                                if (targetseg['memsz'] > seg['memsz']):
                                    post_start = self.align(new_seg['paddr'] + new_seg['memsz'], targetseg['align'])
                                    end_adr = targetseg['paddr'] + targetseg['memsz']
                                    targetseg['offset'] = self.get_next_offset_aligned(dest_metadata, targetseg['align'])
                                    targetseg['vaddr'] = post_start
                                    targetseg['paddr'] = post_start
                                    targetseg['memsz'] = end_adr - targetseg['paddr']
                                    self.add_segment_in_header(dest_metadata);
                                else:
                                    dest_metadata['segments'].remove(targetseg)
                                return dest_metadata
                    else:
                        last_smaller_seg = i
                            
            # address does not fit in one of the segments, create a new one
            new_seg = seg.copy()
            new_seg['offset'] = self.get_next_offset_aligned(dest_metadata, new_seg['align'])
            dest_metadata['segments'].insert(last_smaller_seg+1, new_seg)
            self.add_segment_in_header(dest_metadata);
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

