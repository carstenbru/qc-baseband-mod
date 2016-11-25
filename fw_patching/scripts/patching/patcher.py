#!/usr/bin/env python2

"""
file: patcher.py

Generates a list of needed patches and applies them to the firmware ELF

uses:
    pycparser:      https://github.com/eliben/pycparser
    pycparserext:   https://github.com/inducer/pycparserext

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
import time
import glob
import commands

from binary_patcher import *

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

def read_symtab(filename):
    """
    reads in the symbol table of an ELF file as dictonary
    
    :param filename: ELF file name
    """
    symtab = {}
    table = commands.getstatusoutput('hexagon-readelf -s --wide ' + filename)
    for line in table[1].split('\n'):
        tokens = line.split()
        if (len(tokens) >= 8):
            if (tokens[1] != "Value"):
                symtab[tokens[7]] = int(tokens[1], 16)
    return symtab

def resolve_symbol(symbol, symtab):
    """
    resolves a symbol to its value
    
    :param symbol: name of the symbol to resolve
    :param symtab: symbol table
    """
    res = ""
    try:
        res = symtab[symbol]
    except:
        print "error: symbol is not defined in symbol table: %s" % (symbol)
        exit(1)
    return res

class FuncDefVisitor(c_ast.NodeVisitor):
    """
    parser visitor to handle processing of function definitions
    """
    def __init__(self):
        self.patches = [];
    
    def visit_FuncDef(self, node):
        for spec in node.decl.funcspec:
            for expr in spec.exprlist.exprs:
                #check for "overwrite" attribute and handle it
                if (expr.name.name == "overwrite"):
                    destName = expr.args.exprs[0].value[1:-1]
                    pos = resolve_symbol(destName, symtab)
                    pointer = resolve_symbol(node.decl.name, symtab)
                    
                    self.patches.append(JumpPatch(pos, pointer))
                    print('function "%s" should overwrite "%s"' % (node.decl.name, destName))
                    print('-> function "0x%X" should overwrite "0x%X"' % (pointer, pos))
                #check for "pointer_table" attribute and handle it
                elif (expr.name.name == "pointer_table"):
                    destName = expr.args.exprs[0].value[1:-1]
                    destOffset = int(expr.args.exprs[1].value, 0)
                    pos = resolve_symbol(destName, symtab) + (destOffset << 2)
                    pointer = resolve_symbol(node.decl.name, symtab)
                    self.patches.append(GenericPatch4(pos, pointer))
                    print('function "%s" should be placed in pointer table "%s" at %d' % (node.decl.name, destName, destOffset))
                    print('-> function "0x%X" should be placed in pointer table at "0x%X"' % (pointer, pos))

def generate_patch_list(symtab, fw_wrapper, src_dir):
    """
    generates a list of patches needed to be applied in order to patch the ELF file
    
    :param symtab: symbol table
    :param fw_wrapper: firmware wrapper header file
    :param src_dir: directory containing patch source code
    """
    patches = []
    for filename in glob.glob(src_dir + "/*.c"):
        text = preprocess_file(filename, 'hexagon-cpp', '-DFW_WRAPPER="' + fw_wrapper + '"')

        parser = GnuCParser()
        ast = parser.parse(text, filename)
    
        v = FuncDefVisitor()
        v.visit(ast)
        
        patches.extend(v.patches)
    return patches

	
def generate_version_string_patches(fw_name, symtab):
    """
    generates a list of patches to patch the version, date and time strings
    
    :param fw_name: new firmware name string
    :param symtab: parsed symbol table
    """
    return [
        StringPatch(resolve_symbol("fw_version_string", symtab), fw_name),
	StringPatch(resolve_symbol("fw_time_string", symtab), time.strftime("%H:%M:%S")),
	StringPatch(resolve_symbol("fw_time_string2", symtab), time.strftime("%H:%M:%S")),
	StringPatch(resolve_symbol("fw_date_string", symtab), time.strftime("%d.%m.%Y")),
	StringPatch(resolve_symbol("fw_date_string2", symtab), time.strftime("%d.%m.%Y")),
        ]

def usage():
  print "Usage: %s <base-elf> <patch-elf> <dest-elf> <FW_WRAPPER> <src-dir> <fw_version_string>" % sys.argv[0]
  exit(1)
	
if __name__ == "__main__":
    """
    main function: create and apply all needed patches
    """
    if len(sys.argv) != 7:
        usage()
        
    patches = [ElfPatch(sys.argv[2])]
    
    symtab = read_symtab(sys.argv[2])
    patches.extend(generate_patch_list(symtab, sys.argv[4], sys.argv[5]))
    if (sys.argv[6] != ""):
        patches.extend(generate_version_string_patches(sys.argv[6], symtab))
    patch_firmware(sys.argv[1], sys.argv[3], patches)
