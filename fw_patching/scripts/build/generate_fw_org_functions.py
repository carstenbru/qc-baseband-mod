#!/usr/bin/env python2

"""
file: generate_fw_org_functions.py

Script to generate functions to call an original (and overwritten) function in the firmware

Three files are generated:
	fw_org_functions.c:	function definitions
	fw_org_functions.h:	function declartations header
	fw_org_functions.lcs:	linker script defining symbols used in function definitions
uses:
    hexagondisasm   https://github.com/programa-stic/hexag00n
    pycparser:      https://github.com/eliben/pycparser
    pycparserext:   https://github.com/inducer/pycparserext

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import sys
from sets import Set

from symtab import *
from elf_functions import *

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

from hexagondisasm.disassembler import HexagonDisassembler

org_func_suffix = "_fw_org"

class FuncCallVisitor(c_ast.NodeVisitor):
    """
    parser visitor to handle processing of function calls
    """
    def __init__(self):
        self.req_org_funcs = Set()
        
    def get_req_org_funcs(self):
        """
        returns the list of functions required to generate
        """
        return self.req_org_funcs

    def visit_FuncCall(self, node):
        if (type(node.name) is c_ast.UnaryOp):
            return
        if (type(node.name.name) is c_ast.ArrayRef):
            return
        if (node.name.name.endswith(org_func_suffix)):
            self.req_org_funcs.add(node.name.name)
        if (node.name.name == "__builtin_apply"):
            func_name = node.args.exprs[0].expr.name
            if (func_name.endswith(org_func_suffix)):
                self.req_org_funcs.add(func_name)
 
def generate_function(org_func_name, org_func, symtab, base_elf, metadata, func_symtab):
    """
    generates a helper function to call the original version of an overwritten firmware function
    
    :param org_func_name:   name of the new function
    :param org_func:        original function name, the one we replaced
    :param symtab:          symbol table
    :param base_elf:        base firmware ELF file
    :param metadata:        base firmware metadata
    :param func_symtab:     table of new symbols needed by generated functions
    """
    addr_str, ret_type, param_str = resolve_symbol_all(org_func, symtab)
    address = int(addr_str, 0)
    
    # read first 4 instruction of destination function
    elf_pos, elf_blob = get_offset_in_elf(metadata, address)
    base_elf.seek(elf_pos)
    data = struct.unpack("<IIII", base_elf.read(16))
    
    disasm = HexagonDisassembler(objdump_compatible=True)
    disasm0 = HexagonDisassembler(objdump_compatible=True)
    next_prefix = ""
    
    # generate function start text
    function_decl = "%s %s(%s)" % (ret_type, org_func_name, param_str)
    function_def = "%s {\n\tasm(\n" % function_decl
    function_decl += ";"
    
    # iterate over the four instructions fetched
    packet_size = 0;
    for pos in range(0, 4):
        packet_size += 1
        # disassemble instruction with correct position
        hi = disasm.disasm_one_inst(data[pos], address+pos*4)
        # disassemble instruction again with position 0 to check for PC relative immediates
        hi0 = disasm0.disasm_one_inst(data[pos], pos*4)
        
        if (hi.immext is not None):
            next_prefix = "{ "
            continue
            
        
        disasm_output = hi.text.strip()
        disasm_output_hi0 = hi0.text.strip()
        # if we have a realtive immediate
        if (disasm_output != disasm_output_hi0):
            # loop over all immediates
            for pos_imm, imm in enumerate(hi.imm_ops):
                imm0 = hi0.imm_ops[pos_imm]
                # check for difference -> PC relative immediate
                if (imm0 != imm):
                    # generate a dummy symbol at the destination address 
                    # and write our target relative to this, by this we 
                    # force the compiler to generate a relocation for 
                    # the immediate for us
                    dest_address = address + imm0.value
                    symbol = "sym_0x%X" % dest_address
                    func_symtab[symbol] = dest_address
                    rel_adr = "%s" % symbol
                    disasm_output = disasm_output.replace(("0x%X" % imm.value).lower(), rel_adr)
            
        function_def += '\t\t"%s%s\\n\\t" \n' % (next_prefix, disasm_output)
        next_prefix = ""
        
        if (hi.end_packet):
            break
    
    # generate a jump instruction to the start of remaining original function
    org_func_start = (address + (packet_size << 2))
    symbol = "sym_0x%X" % org_func_start
    func_symtab[symbol] = org_func_start
    function_def += '\t\t"{ jump %s }"\n' % symbol
    function_def += "\t);\n}"
    return function_decl, function_def, func_symtab
 
def write_functions(req_org_funcs, org_functions_file, org_functions_header, org_functions_lcs, symtab, base_elf):
    """
    generates and writes a set of helper functions
    
    :param req_org_funcs:           list of functions to generate
    :param org_functions_file:      destination file for generated c-code
    :param org_functions_header:    destination file for generated header code
    :param org_functions_lcs:       destination file for generated linker symbol definitions
    :param symtab:                  symbol table
    :param base_elf:                base firmware ELF file
    """
    metadata = parse_metadata(base_elf)
    func_symtab = {}
    # write preambles
    org_functions_header.write('#include FW_IMG_WRAPPER\n\n')
    org_functions_file.write('#pragma GCC diagnostic ignored "-Wreturn-type"\n\n')
    # loop over all required functions and generate the c-code and header file
    for func in req_org_funcs:
        org_func = func[:-len(org_func_suffix)]
        print "need %s for function: %s" % (func, org_func)
        org_func_decl, org_func_def, func_symtab = generate_function(func, org_func, symtab, base_elf, metadata, func_symtab)
        org_functions_file.write(org_func_def)
        org_functions_file.write("\n\n")
        org_functions_header.write(org_func_decl)
        org_functions_header.write("\n")
    # generate linker file with symbol definitions
    for sym in func_symtab:
        org_functions_lcs.write('%s = 0x%X;\n' % (sym, func_symtab[sym]))

def generate_org_fw_functions(src_files, org_functions_file, org_functions_header, org_functions_lcs, symtab, base_elf):
    """
    determines the helper functions required to generate and writes them
    
    :param src_files:               list of source code files
    :param org_functions_file:      destination file for generated c-code
    :param org_functions_header:    destination file for generated header code
    :param org_functions_lcs:       destination file for generated linker symbol definitions
    :param symtab:                  symbol table
    :param base_elf:                base firmware ELF file
    """
    # determine required original function helpers
    req_org_funcs = Set()
    for src_file in src_files:
        text = preprocess_file(src_file, 'hexagon-cpp')

        parser = GnuCParser()
        ast = parser.parse(text, src_file)
    
        v = FuncCallVisitor()
        v.visit(ast)
        
        req_org_funcs = req_org_funcs.union(v.get_req_org_funcs())
    
    # generate and write the functions
    write_functions(req_org_funcs, org_functions_file, org_functions_header, org_functions_lcs, symtab, base_elf)
    
def usage():
  print "Usage: %s <org-functions-file> <org-functions-header> <org-functions-lcs> <symtab-json-file> <base-elf> <src-file> [src-file2..]" % sys.argv[0]
  exit(1)
  
if __name__ == "__main__":
    """
    script entry point
    """
    if len(sys.argv) < 7:
        usage()
        
    symtab = read_symtab_json(sys.argv[4])
    org_functions_file = open(sys.argv[1], 'w')
    org_functions_header = open(sys.argv[2], 'w')
    org_functions_lcs = open(sys.argv[3], 'w')
    base_elf = open(sys.argv[5], 'rb')
    generate_org_fw_functions(sys.argv[6:], org_functions_file, org_functions_header, org_functions_lcs, symtab, base_elf)
