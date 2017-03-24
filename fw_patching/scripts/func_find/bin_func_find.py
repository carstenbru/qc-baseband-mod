#!/usr/bin/env python2

"""
file: bin_func_find.py

Script to find back a function of a base file (ELF file or object file) in a target ELF file

uses:
    hexagondisasm   https://github.com/programa-stic/hexag00n

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
import commands
import os

from symtab import *
from elf_functions import *

from hexagondisasm.disassembler import HexagonDisassembler
from hexagondisasm.common import InstructionTemplate

def find_function_in_target(target_file_data, search_data, search_mask, max_results):
    """
    finds a binary search back in target binary data

    :param target_file_data:    target file bytes
    :param search_data:         bytes to search for
    :param search_mask:         mask which will be applied (ORed) to target bytes, to mask out immediates
    :param max_results:         maximum number of results to return, the search is stopped if enough results were found
    """
    res = []
    # go over all possible starting positions for our search data in the target data
    for pos in range(0, len(target_file_data) - len(search_data)):
        match = True
        # match all bytes of the search data
        for i in range(0, len(search_data)):
            # apply mask, no match if byte unequal
            if ((target_file_data[pos + i] | search_mask[i]) != search_data[i]):
                match = False
                break
        if (match): # if we have a match add it to results
            res.append(pos * 4)
            if (len(res) >= max_results):
                return res               
                
    return res

def generate_search_mask(reference_file, func_offset, length, disasm):
    """
    generates a binary mask to mask out immediates in the search
    1s in the mask bits should be ignored when matching
    in addition the base function data is returned

    :param reference_file:  handle to base file
    :param func_offset:     offset in bytes of the function of interest
    :param length number:   of bytes the mask should contain
    :param disasm:          HexagonDisassembler object to use, only passed to avoid new construction in each call
    """
    TEST_ADR = 0
    
    search_mask = []
    search_data = []
    # read base funcion bytes
    reference_file.seek(func_offset)
    read_data = struct.unpack("<%dI" % length, reference_file.read(length * 4))
    for instr_count in range(0, length):
        cur_data = read_data[instr_count]
        # disassemble instruction (with test position, position can be set to arbitrary values, should only be continous for the next instructions)
        hi = disasm.disasm_one_inst(cur_data, TEST_ADR + instr_count*4)
                            
        mask = 0
        if (hi.immext is not None): # set fixed mask for immext instruction
            mask = 0xFFF3FFF
        else: # for other instructions generate mask based on instruction encoding
            if 'i' in hi.template.encoding.fields:
                mask = hi.template.encoding.fields['i'].mask
        
        # append data and mask to result values
        search_data.append(cur_data | mask)
        search_mask.append(mask)
    
    return search_data, search_mask
 
def get_offset_in_reference_file(reference_file_name, func_name_adr):
    """
    finds the offset of a function in a base file, search by address is supported for ELF files and seach by name for object files

    :param reference_file_name: name of the base file
    :param func_name_adr:       address OR name of the function of interest
    """
    try: # find by function address
        address = int(func_name_adr, 0)
        reference_file = open(reference_file_name, 'rb')
        ref_metadata = parse_metadata(reference_file)
        return get_offset_in_elf(ref_metadata, address)[0]
    except ValueError: # find by function name
        # read ELF section table, search for section in form ".text.FUNC_NAME"
        section_descr = commands.getstatusoutput('hexagon-readelf -S --wide %s | grep " .text.%s"' % (reference_file_name, func_name_adr))
        # parse output of hexagon-readelf, extract function offset
        tokens = section_descr[1].split()
        if (len(tokens) < 6):
            return 0
        pos = 5 if (tokens[0] == "[") else 4
        return int(tokens[pos], 16)
    
def file_offset_to_elf(metadata, offsets):
    """
    converts a list of offsets in an ELF file to memory addresses

    :param metadata:    ELF file metadata (parsed)
    :param offsets:     list of offsets in the ELF file to convert
    """
    res = []
    for offset in offsets:
        res.append(get_address_from_offset_in_elf(metadata, offset)[0])
    return res
    
def bin_func_find(base_file_name, target_file_name, function_name_adr, max_results):
    """
    converts a list of offsets in an ELF file to memory addresses

    :param base_file_name:      name of the base input file (ELF or object file)
    :param target_file_name:    name of the target ELF file
    :param func_name_adr:       address OR name of the function of interest
    :param max_results:         maximum number of results to return, if the search could not determine less matches an empty array is returned
    """
    # check input files
    reference_file = open(base_file_name, 'rb')
    search_file = open(target_file_name, 'rb')
    if (not is_elf(reference_file) or not is_elf(search_file)):
        print "error: at least one of the input files is not an ELF file"
        return -1
    
    ref_offset = get_offset_in_reference_file(base_file_name, function_name_adr)
    
    if (ref_offset == 0):
        print "error: could not find function in base file"
        return -2
        
    # determine target file length, read it and parse its metadata
    search_file.seek(0, os.SEEK_END)
    search_file_length = search_file.tell()
    search_file_length -= search_file_length % 4
    search_file.seek(0)
    search_file_data = struct.unpack("<%dI" % (search_file_length / 4), search_file.read(search_file_length))
    search_file.seek(0)
    search_file_metadata = parse_metadata(search_file)
    search_file.close()
    
    # determine base file length
    reference_file.seek(0, os.SEEK_END)
    max_size = reference_file.tell() - ref_offset
    
    min_size = 2
    cur_size = 2
    
    print "searching for function %s" % function_name_adr
    
    disasm = HexagonDisassembler(objdump_compatible=True)

    # loop to search with different search string lengths in order to try to find a unique match
    while (True):
        print "current search string length: %d min length: %s max length: %s" % (cur_size, min_size, max_size)
        
        # generate search data and perform the search
        search_data, search_mask = generate_search_mask(reference_file, ref_offset, cur_size, disasm)
        res = find_function_in_target(search_file_data, search_data, search_mask, 2)
        num_res = len(res)
        
        if (num_res == 1): # unique match: we are done!
            phys_adr = file_offset_to_elf(search_file_metadata, res)
            print "found function %s of base file at 0x%08X of target" % (function_name_adr, phys_adr[0])
            return phys_adr
            
        if (num_res > 1): # multiple matches
            if (cur_size >= max_size): # stop if we reached the maximum size..we cannot get better
                print "could not identify function, multiple matches for maximal size"
                res = find_function_in_target(search_file_data, search_data, search_mask, max_results + 1)
                if (len(res) <= max_results):
                    return file_offset_to_elf(search_file_metadata, res)
                else:
                    return []
            # compute search data length for next iteration
            min_size = cur_size + 1
            cur_size *= 2
            if (cur_size > max_size):
                cur_size = max_size
        
        if (num_res == 0): # no match
            if (cur_size <= min_size): # stop if we reached minimal size, we cannot find a unique match
                print "could not identify function, no matches for minimal size"
                # regenerate last result with multiple macthes (cur_size-1=
                search_data, search_mask = generate_search_mask(reference_file, ref_offset, cur_size - 1, disasm)
                res = find_function_in_target(search_file_data, search_data, search_mask, max_results + 1)
                if (len(res) <= max_results):
                    return file_offset_to_elf(search_file_metadata, res)
                else:
                    return []
            # compute search data length for next iteration
            max_size = cur_size - 1
            cur_size = ((max_size - min_size)) / 2 + min_size

def usage():
  print "Usage: %s <base-functions-file> <target-function-file> <function name or address>" % sys.argv[0]
  exit(1)
  
if __name__ == "__main__":
    """
    script entry point
    """
    if len(sys.argv) < 4:
        usage()
        
    MAX_RESULTS = 4
        
    print bin_func_find(sys.argv[1], sys.argv[2], sys.argv[3], MAX_RESULTS)
    

    
