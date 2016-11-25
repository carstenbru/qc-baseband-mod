import sys

from pycparser import c_ast, preprocess_file
from pycparserext.ext_c_parser import GnuCParser

def usage():
  print "Usage: %s <path-to/fw_wrapper.h> <output-lcs>" % sys.argv[0]
  exit(1)
  
class DeclVisitor(c_ast.NodeVisitor):
    def __init__(self, lcs_file):
        self.lcs_file = lcs_file;
    
    def visit_Decl(self, node):
        for spec in node.funcspec:
            for expr in spec.exprlist.exprs:
                if (expr.name.name == "address"):
                    lcs_file.write('%s = %s;\n' % (node.name, expr.args.exprs[0].value))
  
def generate_wrapper_lcs(filename, lcs_file):
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
