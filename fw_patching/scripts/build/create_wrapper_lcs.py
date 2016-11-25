#!/usr/bin/env python2

"""
file: create_wrapper_lcs.py

Script to generate wrapper linker script (fw_wrapper.lcs) from firmware wrapper header.
The generated linker script contains the symbol definitions for symbols of the base firmware.

uses:
    pycparser:      https://github.com/eliben/pycparser
    pycparserext:   https://github.com/inducer/pycparserext

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

def usage():
  print "Usage: %s <path-to/fw_wrapper.h> <output-lcs>" % sys.argv[0]
  exit(1)
  
class DeclVisitor(c_ast.NodeVisitor):
    """
    parser visitor to handle processing of declarations
    """
    def __init__(self, lcs_file):
        self.lcs_file = lcs_file;
    
    def visit_Decl(self, node):
        for spec in node.funcspec:
            for expr in spec.exprlist.exprs:
                #check for "address" attribute and handle it
                if (expr.name.name == "address"):
                    lcs_file.write('%s = %s;\n' % (node.name, expr.args.exprs[0].value))
  
def generate_wrapper_lcs(filename, lcs_file):
    """
    main linker script generation function
    
    :param filename: firmware wrapper header
    :param lcs_file: destination file
    """
    text = preprocess_file(filename, 'hexagon-cpp')

    parser = GnuCParser()
    ast = parser.parse(text, filename)
    
    v = DeclVisitor(lcs_file)
    v.visit(ast)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        usage()
        
    lcs_file = open(sys.argv[2], 'w');
    generate_wrapper_lcs(sys.argv[1], lcs_file)
