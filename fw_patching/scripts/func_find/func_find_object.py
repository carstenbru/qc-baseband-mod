#!/usr/bin/env python2

"""
file: func_find_object.py

Script to find all functions of a base object file in a target ELF binary

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
import commands

from bin_func_find import *

def find_functions(reference_file_name):
    """
    generates a list of all functions in an object file
    
    :param reference_file_name: name of the new function
    """
    res = []
    section_descr = commands.getstatusoutput('hexagon-readelf -S --wide %s | grep " .text."' % (reference_file_name))
    # parse output of commandline command
    tokens = section_descr[1].split()
    for s in tokens:
        if (len(s) > 6):
            if (s[0:6] == ".text."):
                res.append(s[6:])
    return res

def func_find_object(base_object_file, target_file_name):
    """
    finds all functions of a base object file in a target ELF file
    
    :param base_object_file: name of the base object file
    :param target_file_name: name of the target ELF file
    """
    MAX_RESULTS = 4
    res = {}
    
    functions = find_functions(base_object_file)
    # iterate over all functions
    for function in functions:
        adr = bin_func_find(base_object_file, target_file_name, function, MAX_RESULTS)
        res[function] = adr
            
    return res

def summarize_results(results):
    find_functions(reference_file_name):
    """
    prints a summary of the search results
    
    :param results: obtained results
    """
    print "\n------- results -------"
    for key, value in results.iteritems():
        addresses = ""
        for v in value:
            addresses += ", 0x%08X" % v
        print "%s at [%s]" % (key, addresses[2:])

def usage():
    print "Usage: %s <base-object-file> <target-firmware-elf>" % sys.argv[0]
    exit(1)
  
if __name__ == "__main__":
    """
    script entry point
    """
    if len(sys.argv) < 3:
        usage()
        
    results = func_find_object(sys.argv[1], sys.argv[2])
    summarize_results(results)
