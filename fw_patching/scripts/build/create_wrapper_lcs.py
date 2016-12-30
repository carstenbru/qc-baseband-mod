#!/usr/bin/env python2

"""
file: create_wrapper_lcs.py

Script to generate wrapper linker script (fw_wrapper.lcs) from firmware wrapper header.
The generated linker script contains the symbol definitions for symbols of the base firmware.
In addition the symbol table is written in JSON format to enable easy reading in later steps.

uses:
    pycparser:      https://github.com/eliben/pycparser
    pycparserext:   https://github.com/inducer/pycparserext

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
import json

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

def usage():
  print "Usage: %s <path-to/fw_wrapper.h> <output-lcs> <output-symtab-json>" % sys.argv[0]
  exit(1)
  
class DeclVisitor(c_ast.NodeVisitor):
    """
    parser visitor to handle processing of declarations
    """
    def __init__(self, lcs_file):
        self.lcs_file = lcs_file
        self.symtab = {}
        
    def get_symtab(self):
        """
        returns the generated symbol table
        """
        return self.symtab
    
    def type_to_str(self, type_def):
        """
        converts a type definition into a string representation
        
        :param type_def: type definition
        """
        ptr_str = ""
        while (type(type_def) is c_ast.PtrDecl):
            type_def = type_def.type
            ptr_str += "*"
        res = ""
        for qual in type_def.quals:
            res += "%s " % qual
        for s in type_def.type.names:
            res = "%s%s " % (res, s)
        res = res[:-1] + ptr_str
        return res, type_def.declname
    
    def param_list_to_str(self, param_list):
        """
        converts a parameter list into a string representation
        
        :param param_list: parameter list
        """
        res = ""
        if (param_list is None):
            return res
        for param in param_list.params:
            if ((type(param) is c_ast.Decl) | (type(param) is c_ast.Typename)):
                ptype, name = self.type_to_str(param.type)
                res = "%s, %s %s" % (res, ptype, name)
            elif (type(param) is c_ast.EllipsisParam):
                res = "%s, ..." % res
            else:
                print "error: unknown type in parsing"
                print param
        return res[2:]
    
    def visit_Decl(self, node):
        """
        handle processing of declarations
        """
        for spec in node.funcspec:
            for expr in spec.exprlist.exprs:
                #check for "address" attribute and handle it
                if (expr.name.name == "address"):
                    lcs_file.write('%s = %s;\n' % (node.name, expr.args.exprs[0].value))
                    ret = ""
                    params = ""
                    if (type(node.type) is c_ast.FuncDecl):                        
                        ret, name = self.type_to_str(node.type.type)
                        params = self.param_list_to_str(node.type.args)
                    self.symtab[node.name] = [expr.args.exprs[0].value, ret, params]
  
def generate_wrapper_lcs(filename, lcs_file, symtab_json_file):
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
    
    json.dump(v.get_symtab(), open(symtab_json_file,'w'))

if __name__ == "__main__":
    """
    script entry point
    """
    if len(sys.argv) != 4:
        usage()
        
    lcs_file = open(sys.argv[2], 'w')
    generate_wrapper_lcs(sys.argv[1], lcs_file, sys.argv[3])
