#!/usr/bin/env python2

# TODO doc

import sys

from symtab import *
from bin_func_find import *

def generate_mapping(base_firmware_elf, target_firmware_elf, symtab_file):
    MAX_RESULTS = 4
    
    symtab = read_symtab_json(symtab_file)
    
    mapping = {}
    for key, value in symtab.iteritems():
        if (value[0] == '"unknown"'):
                continue
        base_adr = int(value[0], 0)
        if (value[1] != ""): # only search for functions, no other symbols
            res = bin_func_find(base_firmware_elf, target_firmware_elf, value[0], MAX_RESULTS)
            mapping[base_adr] = (1, res)
        else:
            mapping[base_adr] = (0, [])
            
    return mapping

def write_sed_script(mapping, sed_script_filename):
    sed_script = open(sed_script_filename, 'w')
    for key, value in mapping.iteritems():
        addresses = value[1]
        if (len(addresses) == 1):
            new_adr = "0x%08X" % addresses[0]
        else:
            if (len(addresses) == 0):
                new_adr = '"unknown"'
            else:
                candidates = ""
                for adr in addresses:
                    candidates += ", 0x%08X" % adr
                new_adr = '"unknown" \/* candidates: %s *\/' % candidates[2:]
            
        sed_script.write("s/0x%08X/%s/g\n" % (key, new_adr))
        sed_script.write("s/0x%X/%s/g\n" % (key, new_adr))
        sed_script.write("s/0x%08x/%s/g\n" % (key, new_adr))
        sed_script.write("s/0x%x/%s/g\n" % (key, new_adr))
    sed_script.close()
    
def print_stats(mapping):
    total_functions = 0
    found_functions = 0
    candidates_results = 0
    for key, value in mapping.iteritems():
        if (value[0] == 1):
            total_functions += 1
            if (len(value[1]) == 1):
                found_functions += 1
            if (len(value[1]) > 1):
                candidates_results += 1
    print "\n------- wrapper transfer statistics -------"
    print "Total number of defined symbols: %s" % len(mapping)
    print "Total number of functions: %s" % total_functions
    print "Found functions: %s" % found_functions
    print "Functions with multiple candidates: %s" % candidates_results
    print "Not identified functions (not uniquely detected): %s" % (total_functions - found_functions)
    print "Not found functions (no match at all / too many candidates): %s\n" % (total_functions - found_functions - candidates_results)

def usage():
    print "Usage: %s <base-firmware-elf> <target-firmware-elf> <base-fw-symtab> <sed-script-output>" % sys.argv[0]
    exit(1)
  
if __name__ == "__main__":
    """
    script entry point
    """
    if len(sys.argv) < 5:
        usage()
        
    mapping = generate_mapping(sys.argv[1], sys.argv[2], sys.argv[3])
    write_sed_script(mapping, sys.argv[4])
    
    print_stats(mapping)


                
