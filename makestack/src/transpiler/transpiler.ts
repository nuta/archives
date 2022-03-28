import * as assert from "assert";
import * as parser from "@babel/parser";
import traverse, { NodePath } from "@babel/traverse";
import * as t from "@babel/types";
import { TranspileError, UnimplementedError } from ".";

function isRequireCall(node: t.Expression | null, pkg: string) {
    if (t.isCallExpression(node)
        && t.isIdentifier(node.callee)
        && node.callee.name == "require"
        && node.arguments.length == 1
    ) {
        const arg = node.arguments[0]
        return t.isStringLiteral(arg) && arg.value == pkg;
    }

    return false;
}

function isDeviceContextAPICall(apiVarName: string | null, node: t.Node): node is t.ExpressionStatement {
    const deviceContextCallbacks = [
        "onReady",
    ];

    return t.isExpressionStatement(node)
        && t.isCallExpression(node.expression)
        && t.isMemberExpression(node.expression.callee)
        && !node.expression.callee.computed
        && t.isIdentifier(node.expression.callee.object)
        && t.isIdentifier(node.expression.callee.property)
        && node.expression.callee.object.name == apiVarName
        && deviceContextCallbacks.includes(node.expression.callee.property.name);
    }

export class Transpiler {
    private lambda: string = "";
    private setup: string = "";
    private apiVarName: string | null = null;
    private funcNameStack: string[] = ["(top level)"];

    public transpile(code: string): string {
        const ast = parser.parse(code);
        traverse(ast, {
            Program: (path: NodePath) => {
                if (path.isProgram(path.node)) {
                    for (const stmt of path.node.body) {
                        this.visitTopLevel(stmt);
                    }
                }
            }
        });

        return this.lambda + "\n\nvoid app_setup(Context *__ctx) {\n" + this.setup + "}\n";
    }

    private getCurrentFuncName(): string {
        const name = this.funcNameStack[this.funcNameStack.length - 1];
        assert(name);
        return name;
    }

    private getNameFromVarDeclId(decl: t.Node): string {
        if (t.isIdentifier(decl)) {
            return decl.name;
        }

        throw new TranspileError(decl, `expected an identifier`);
    }

    private visitBlockStmt(stmt: t.BlockStatement): string {
        const stmts = stmt.body.map(stmt => this.visitStmt(stmt));
        return `{\n${stmts.join(";\n")};\n}`;
    }

    private visitFunctionBody(stmt: t.BlockStatement): string {
        const stmts = stmt.body.map(stmt => this.visitStmt(stmt));
        return `{\n${stmts.join(";\n")};\nreturn VM_UNDEF;\n}`;
    }

    private visitExprStmt(stmt: t.ExpressionStatement): string {
        return this.visitExpr(stmt.expression);
    }

    private visitVarDecl(decl: t.VariableDeclarator): string {
        let init;
        if (decl.init) {
            init = this.visitExpr(decl.init);
        } else {
            init = "VM_UNDEF";
        }

        const id = this.getNameFromVarDeclId(decl.id);
        return `VM_SET("${id}", ${init})`;
    }

    private visitVarDeclStmt(stmt: t.VariableDeclaration): string {
        return stmt.declarations
            .map((decl) => this.visitVarDecl(decl))
            .join(";");
    }

    private visitBlockOrExpr(blockOrExpr: t.Node): string {
        if (t.isBlockStatement(blockOrExpr)) {
            return this.visitBlockStmt(blockOrExpr);
        } else if (t.isExpressionStatement(blockOrExpr)) {
            return this.visitExprStmt(blockOrExpr) + ";";
        } else {
            throw new UnimplementedError(blockOrExpr);
        }
    }

    private visitIfStmt(stmt: t.IfStatement): string {
        let code = "";
        let ifType = "if";
        let ifStmt = stmt;
        while (ifStmt) {
            const test = this.visitExpr(ifStmt.test);
            const body = this.visitBlockOrExpr(ifStmt.consequent);
            code += `${ifType} (${test}) ${body}`;
            if (ifStmt.alternate) {
                if (t.isIfStatement(ifStmt.alternate)) {
                    ifType = "else if";
                    ifStmt = ifStmt.alternate;
                } else {
                    code += "else " + this.visitBlockOrExpr(ifStmt.alternate);
                    break;
                }
            } else {
                break;
            }
        }

        return code;
    }

    private visitWhileStmt(stmt: t.WhileStatement): string {
        return "while (" + this.visitExpr(stmt.test) + ")" + this.visitBlockOrExpr(stmt.body);
    }

    private visitDoWhileStmt(stmt: t.DoWhileStatement): string {
        const test = this.visitExpr(stmt.test);
        const body = this.visitBlockOrExpr(stmt.body);
        return `do ${body} while (${test});`;
    }

    private visitForStmt(stmt: t.ForStatement): string {
        let init;
        if (t.isVariableDeclaration(stmt.init)) {
            init = this.visitStmt(stmt.init);
        } else {
            init = this.visitExpr(stmt.init as t.Node);
        }

        const test = this.visitExpr(stmt.test as t.Node);
        const update = this.visitExpr(stmt.update as t.Node);
        const body = this.visitBlockOrExpr(stmt.body);
        return `${init};\nfor (; ${test}; ${update}) ${body}`;
    }

    private visitBreakStmt(stmt: t.BreakStatement): string {
        if (stmt.label) {
            throw new UnimplementedError(stmt.label);
        }

        return `break;`;
    }

    private visitContinueStmt(stmt: t.ContinueStatement): string {
        if (stmt.label) {
            throw new UnimplementedError(stmt.label);
        }

        return `continue;`;
    }

    private visitReturnStmt(stmt: t.ReturnStatement): string {
        const value = this.visitExpr(stmt.argument as t.Node);
        return `return ${value};`;
    }

    private visitStmt(stmt: t.Statement): string {
        if (t.isExpressionStatement(stmt)) {
            return this.visitExprStmt(stmt);
        } else if (t.isVariableDeclaration(stmt)) {
            return this.visitVarDeclStmt(stmt);
        } else if (t.isIfStatement(stmt)) {
            return this.visitIfStmt(stmt);
        } else if (t.isWhileStatement(stmt)) {
            return this.visitWhileStmt(stmt);
        } else if (t.isDoWhileStatement(stmt)) {
            return this.visitDoWhileStmt(stmt);
        } else if (t.isForStatement(stmt)) {
            return this.visitForStmt(stmt);
        } else if (t.isBreakStatement(stmt)) {
            return this.visitBreakStmt(stmt);
        } else if (t.isContinueStatement(stmt)) {
            return this.visitContinueStmt(stmt);
        } else if (t.isReturnStatement(stmt)) {
            return this.visitReturnStmt(stmt);
        } else {
            throw new UnimplementedError(stmt);
        }
    }

    private visitCallExpr(expr: t.CallExpression): string {
        const callee = this.visitExpr(expr.callee);
        const func = this.getCurrentFuncName();
        const line = (expr.loc) ? expr.loc.start.line : -1;
        const args = expr.arguments.map(arg => this.visitExpr(arg));
        const callArgs = [
            (func == "(anonymous function)") ? `VM_ANON_LOC(${line})` : `VM_APP_LOC("${func}", ${line})`,
            callee,
            args.length,
            ...args
        ];

        return `VM_CALL(${callArgs.join(", ")})`;
    }

    private lambdaId: number = 0;
    private visitArrowFuncExpr(func: t.ArrowFunctionExpression): string {
        if (func.generator) {
            throw new TranspileError(func, "Generator is not supported.");
        }

        if (func.async) {
            throw new TranspileError(func, "Async function is not supported.");
        }

        const uniqueId = this.lambdaId;
        const lambdaName = `__lambda_${uniqueId}`;
        const closureName = `__closure_${uniqueId}`;
        this.lambdaId++;

        const paramNames = [];
        for (const param of func.params) {
            if (t.isIdentifier(param)) {
                paramNames.push(`"${param.name}"`);
            } else {
                throw new UnimplementedError(param);
            }
        }

        this.funcNameStack.push("(anonymous function)");
        let body;
        if (t.isBlockStatement(func.body)) {
            body = this.visitFunctionBody(func.body);
        } else {
            throw new UnimplementedError(func.body);
        }
        this.funcNameStack.pop();

        let nargs = paramNames.length;
        if (nargs > 6) {
            throw new TranspileError(func, "Too many parameters.");
        }

        const macroArgs = [closureName, ...paramNames];
        const enterMacro = `VM_FUNC_ENTER${nargs}(${macroArgs.join(", ")});`;
        body = body.replace(/^[ \t\n]*\{/, enterMacro);

        this.lambda += `VM_FUNC_DEF(${lambdaName}, ${closureName}) {\n${body}\n\n`;
        return `VM_FUNC(${lambdaName}, ${closureName})`;
    }

    private visitNumberLit(expr: t.NumericLiteral): string {
        if (!Number.isInteger(expr.value)) {
            throw new TranspileError(expr, "non-integer number is not yet supported");
        }

        return `VM_INT(${expr.value})`;
    }

    private visitBooleanLit(expr: t.BooleanLiteral): string {
        return `VM_BOOL(${expr.value ? 'true' : 'false'})`;
    }

    private visitStringLit(expr: t.StringLiteral): string {
        // TODO: escape sequences
        return `VM_STR("${expr.value}")`;
    }

    private visitTemplateLiteral(expr: t.TemplateLiteral): string {
        let tmpl = "";
        let str = true;
        let expr_i = 0;
        let str_i = 0;
        while (expr.quasis[str_i]) {
            if (str) {
                const frag = expr.quasis[str_i++];
                tmpl += `VM_STR("${frag.value.raw}")`;
                tmpl += frag.tail ? "" : " + ";
            } else {
                const exprStr = this.visitExpr(expr.expressions[expr_i++]);
                tmpl += `(${exprStr}) + `;
            }

            str = !str;
        }

        return `(${tmpl})`;
    }

    private visitIdentExpr(expr: t.Identifier): string {
        return `VM_GET("${expr.name}")`;
    }

    private visitMemberExpr(expr: t.MemberExpression): string {
        const obj = this.visitExpr(expr.object);
        let prop;
        if (expr.computed) {
            prop = this.visitExpr(expr.property);
        } else {
            if (!t.isIdentifier(expr.property)) {
                throw new Error("expected identifier");
            }
            prop = `VM_STR("${expr.property.name}")`;
        }

        return `VM_MGET(${obj}, ${prop})`
    }

    private visitUpdateExpr(expr: t.UpdateExpression): string {
        const SUPPORTED_OPS: string[] = ["++", "--"];
        if (!SUPPORTED_OPS.includes(expr.operator)) {
            throw new TranspileError(expr, `\`${expr.operator}' operator is not yet supported.`);
        }

        let arg = this.visitExpr(expr.argument);
        return expr.prefix ? (expr.operator + arg) : (arg + expr.operator);
    }

    private visitLogicalExpr(expr: t.LogicalExpression): string {
        const SUPPORTED_OPS: string[] = [ "||", "&&" ];
        if (!SUPPORTED_OPS.includes(expr.operator)) {
            throw new TranspileError(expr, `\`${expr.operator}' operator is not yet supported.`);
        }

        const left = this.visitExpr(expr.left);
        const right = this.visitExpr(expr.right);
        return `((${left}) ${expr.operator} (${right}))`;
    }

    private visitUnaryExpr(expr: t.UnaryExpression): string {
        const SUPPORTED_OPS: string[] = [ "!", "~", "+", "-" ];
        if (!SUPPORTED_OPS.includes(expr.operator)) {
            throw new TranspileError(expr, `\`${expr.operator}' operator is not yet supported.`);
        }

        return expr.operator + "(" + this.visitExpr(expr.argument) + ")";
    }

    private visitBinaryExpr(expr: t.BinaryExpression): string {
        const SUPPORTED_OPS: string[] = [
            "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=", "&", "|", "^", "<<", ">>", "%",
            "===", "!=="
        ];

        if (!SUPPORTED_OPS.includes(expr.operator)) {
            throw new TranspileError(expr, `\`${expr.operator}' operator is not yet supported.`);
        }

        let op: string = expr.operator;
        if (["===", "!=="].includes(op)) {
            // Replace === and !=== with == and != respectively.
            op = op[0] + "=";
        }

        return "(" + this.visitExpr(expr.left) + op + this.visitExpr(expr.right) + ")";
    }

    private visitAssignExpr(expr: t.AssignmentExpression): string {
        const SUPPORTED_OPS: string[] = [
            "=", "+=", "-=", "*=", "/=",  "&=", "|=", "^=", "<<=", ">>="
        ];

        if (!SUPPORTED_OPS.includes(expr.operator)) {
            throw new TranspileError(expr, `\`${expr.operator}' operator is not yet supported.`);
        }

        if (expr.operator == "=") {
            if (!t.isIdentifier(expr.left)) {
                throw new TranspileError(expr, "The left-hand side of `=' operator must be an identifier.");
            }

            return `VM_SET("${expr.left.name}", ${this.visitExpr(expr.right)})`;
        } else {
            return "(" + this.visitExpr(expr.left) + expr.operator + this.visitExpr(expr.right) + ")";
        }
    }

    private visitConditionalExpr(expr: t.ConditionalExpression): string {
        const test = this.visitExpr(expr.test);
        const trueExpr = this.visitExpr(expr.consequent);
        const falseExpr = this.visitExpr(expr.alternate);
        return `(${test}) ? (${trueExpr}) : (${falseExpr})`;
    }

    private visitExpr(expr: t.Node): string {
        if (t.isNumericLiteral(expr)) {
            return this.visitNumberLit(expr);
        } else if (t.isBooleanLiteral(expr)) {
            return this.visitBooleanLit(expr);
        } else if (t.isStringLiteral(expr)) {
            return this.visitStringLit(expr);
        } else if (t.isTemplateLiteral(expr)) {
            return this.visitTemplateLiteral(expr);
        } else if (t.isIdentifier(expr)) {
            return this.visitIdentExpr(expr);
        } else if (t.isMemberExpression(expr)) {
            return this.visitMemberExpr(expr);
        } else if (t.isCallExpression(expr)) {
            return this.visitCallExpr(expr);
        } else if (t.isUnaryExpression(expr)) {
            return this.visitUnaryExpr(expr);
        } else if (t.isBinaryExpression(expr)) {
            return this.visitBinaryExpr(expr);
        } else if (t.isLogicalExpression(expr)) {
            return this.visitLogicalExpr(expr);
        } else if (t.isUpdateExpression(expr)) {
            return this.visitUpdateExpr(expr);
        } else if (t.isAssignmentExpression(expr)) {
            return this.visitAssignExpr(expr);
        } else if (t.isArrowFunctionExpression(expr)) {
            return this.visitArrowFuncExpr(expr);
        } else if (t.isFunctionExpression(expr)) {
            // XXX:
            return this.visitArrowFuncExpr(expr as any as t.ArrowFunctionExpression);
        } else if (t.isConditionalExpression(expr)) {
            return this.visitConditionalExpr(expr);
        } else {
            throw new UnimplementedError(expr);
        }
    }

    private visitTopLevel(node: t.Statement) {
        // TODO: Support const variables in the top-level.

        // Parse `const apiVarName = require("makestack")` and save the declared
        // identifier in apiVarName.
        if (t.isVariableDeclaration(node)) {
            for (const decl of node.declarations) {
                if (t.isIdentifier(decl.id) && isRequireCall(decl.init, "makestack")) {
                    this.apiVarName = decl.id.name;
                }
            }
        }

        // API calls which registers device-side callbacks. Call
        // t.isMemberExpression() again because tsc does not infer the type.
        if (isDeviceContextAPICall(this.apiVarName, node)
            && t.isCallExpression(node.expression)
            && t.isMemberExpression(node.expression.callee)) {
            // app.onReady(...) => __onReady(...)
            node.expression.callee = t.identifier("__" + node.expression.callee.property.name);
            this.setup += this.visitCallExpr(node.expression) + `;`;
        }
    }
}
