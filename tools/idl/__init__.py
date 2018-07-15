from antlr4 import *
from .parser.idlLexer import idlLexer
from .parser.idlParser import idlParser
from .parser.idlListener import idlListener


class Listener(idlListener):
    def __init__(self):
        self.types = {}
        self.interfaces = []

    def exitAttrList(self, ctx):
        ctx.attrs = {}
        for i in range(0, ctx.getChildCount()):
            child = ctx.getChild(i)
            if isinstance(child, TerminalNode):
                continue

            ctx.attrs[child.name] = child.value

    def exitAttr(self, ctx):
        ctx.name = str(ctx.getChild(0))
        ctx.value = str(ctx.getChild(2))

    def exitInterfaceDef(self, ctx):
        calls = []
        for i in range(3, ctx.getChildCount()):
            child = ctx.getChild(i)
            if child == '}':
                break

            if isinstance(child, TerminalNode):
                continue

            calls.append({
                "name": child.name,
                "attrs": child.attrs[0],
                "args": child.args,
                "rets": child.rets
            })

        self.interfaces.append({
            "name": str(ctx.getChild(2)),
            "attrs": ctx.getChild(0).attrs,
            "calls": calls
        })

    def exitCallDef(self, ctx):
        ctx.name = str(ctx.getChild(1))
        ctx.attrs = ctx.getChild(0).attrs,
        ctx.args = ctx.getChild(3).args
        ctx.rets = ctx.getChild(7).args

    def exitArgList(self, ctx):
        ctx.args = []
        for i in range(0, ctx.getChildCount()):
            child = ctx.getChild(i)
            if child == ')':
                break

            if isinstance(child, TerminalNode):
                continue

            ctx.args.append({
                "name": child.name,
                "type": child.type
            })

    def exitArg(self, ctx):
        ctx.name = str(ctx.getChild(0))
        ctx.type = str(ctx.getChild(2))

    def exitTypeDef(self, ctx):
        self.types.append({
            "new_name": str(ctx.getChild(1)),
            "alias_of": str(ctx.getChild(3))
        })

class ParseError(Exception):
    def __init__(self, line, col, msg):
        self.line = line
        self.col = col
        self.msg = msg

    def __str__(self):
        return f"{self.line}:{self.col}: {self.msg}"

class ErrorListener(error.ErrorListener.ErrorListener):
    def __init__(self):
        super()

    def syntaxError(self, unused1, unused2, line, col, msg, unused):
        raise ParseError(line, col, msg)

def parse(filepath):
    lexer = idlLexer(FileStream(filepath))
    lexer._listeners = [ErrorListener()]
    stream = CommonTokenStream(lexer)
    parser = idlParser(stream)
    parser._listeners = [ErrorListener()]
    tree = parser.idl()
    walker = ParseTreeWalker()
    listener = Listener()
    walker.walk(listener, tree)
    return { "types": listener.types, "interfaces": listener.interfaces }
