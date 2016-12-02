#!/usr/bin/env python2

"""
file: symtab.py

Reads in a symbol table and can resolve symbols

author: Carsten Bruns (carst.bruns@gmx.de)
"""

import commands
import json

def read_symtab_elf(filename):
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
                symtab[tokens[7]] = [int(tokens[1], 16), "", ""]
    return symtab

def read_symtab_json(filename):
    """
    reads in the symbol table from a JSON file as dictonary
    
    :param filename: JSON file name
    """
    return json.load(open(filename))

def resolve_symbol(symbol, symtab):
    """
    resolves a symbol to its value
    
    :param symbol: name of the symbol to resolve
    :param symtab: symbol table
    """
    res = ""
    try:
        res = symtab[symbol][0]
    except:
        print "error: symbol is not defined in symbol table: %s" % (symbol)
        exit(1)
    return res

def resolve_symbol_all(symbol, symtab):
    """
    resolves a symbol to all its stored information, including the value and additional information,
    i.e. return type and parameters for function symbols
    
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
