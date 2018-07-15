grammar idl;
NUM: [0-9]+;
NAME: [a-z][a-zA-Z0-9_]*;
WS: [ \t\r\n]+ -> skip ;

idl: typeDef* interfaceDef*;
attrList: '[' attr (',' attr) * ']';
attr: 'id' '(' NUM ')';
typeDef: 'type' NAME '=' NAME ';';

interfaceDef: attrList 'interface' NAME '{' callDef* '}';
arg: NAME ':' NAME;
argList: (arg  (',' arg) *) ?;
callDef: attrList NAME '(' argList ')' '->' '(' argList ')' ';';
