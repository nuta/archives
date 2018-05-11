from antlr4 import *
from .parser.idlLexer import idlLexer
from .parser.idlParser import idlParser
from .parser.idlListener import idlListener


class Listener(idlListener):
    def __init__(self):
        self.types = []
        self.calls = []

    def exitService(self, ctx):
        self.name = str(ctx.getChild(2))
        self.id = int(str(ctx.getChild(5)))

    def exitCallDef(self, ctx):
        self.calls.append({
            "name": str(ctx.getChild(0)),
            "args": ctx.getChild(2).args,
            "rets": ctx.getChild(6).args
        })

        for i in range(0, ctx.getChildCount()):
            child = ctx.getChild(i)
            if isinstance(child, TerminalNode):
                break

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


def parse(filepath):
    lexer = idlLexer(FileStream(filepath))
    stream = CommonTokenStream(lexer)
    parser = idlParser(stream)
    tree = parser.idl()
    walker = ParseTreeWalker()
    listener = Listener()
    walker.walk(listener, tree)
    return {
        "name": listener.name,
        "id": listener.id,
        "types": listener.types,
        "calls": listener.calls
    }
