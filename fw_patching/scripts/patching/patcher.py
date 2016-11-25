#!/usr/bin/env python2

import sys
import time
import glob
import commands

from binary_patcher import *

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

def read_symtab(filename):
    symtab = {}
    table = commands.getstatusoutput('hexagon-readelf -s ' + filename)
    for line in table[1].split('\n'):
        tokens = line.split()
        if (len(tokens) >= 8):
            if (tokens[1] != "Value"):
                symtab[tokens[7]] = int(tokens[1], 16)
    return symtab

def resolve_symbol(symbol, symtab):
    res = ""
    try:
        res = symtab[symbol]
    except:
        print "error: symbol is not defined in symbol table: %s" % (symbol)
        exit(1)
    return res

class FuncDefVisitor(c_ast.NodeVisitor):
    def __init__(self):
        self.patches = [];
    
    def visit_FuncDef(self, node):
        for spec in node.decl.funcspec:
            for expr in spec.exprlist.exprs:
                if (expr.name.name == "overwrite"):
                    destName = expr.args.exprs[0].value[1:-1]
                    
                    self.patches.append(JumpPatch(resolve_symbol(destName, symtab), resolve_symbol(node.decl.name, symtab)))
                    #print('function "%s" should overwrite "%s"' % (node.decl.name, destName))
                    #print('function "%s" should overwrite "%s"' % (resolve_symbol(node.decl.name, symtab), resolve_symbol(destName, symtab)))

def generate_patch_list(symtab, fw_wrapper, src_dir):
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
    if len(sys.argv) != 7:
        usage()
        
    patches = [ElfPatch(sys.argv[2])]
    
    symtab = read_symtab(sys.argv[2])
    patches.extend(generate_patch_list(symtab, sys.argv[4], sys.argv[5]))
    if (sys.argv[6] != ""):
        patches.extend(generate_version_string_patches(sys.argv[6], symtab))
    patch_firmware(sys.argv[1], sys.argv[3], patches)
