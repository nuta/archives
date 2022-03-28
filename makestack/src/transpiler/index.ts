import * as t from "@babel/types";
import { Transpiler } from "./transpiler";

export class TranspileError extends Error {
    public node: t.Node;
    constructor(node: t.Node, message: string) {
        super(message);
        this.node = node;
    }
}

export class UnimplementedError extends TranspileError {
    constructor(node: t.Node) {
        super(node, `Unimplemented language feature: \`${node.type}'`);
    }
}

export { Transpiler };
