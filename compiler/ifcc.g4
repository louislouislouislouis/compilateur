grammar ifcc;

axiom : prog ;

prog : 'int' 'main' '(' ')' '{' expr+ '}' ;

expr: ret | decl | assign ;

ret : 'return' rval ';';

decl: type sdecl (',' sdecl)* ';' ;
sdecl: ID ('=' rval)? ;

assign: sassign (',' sassign)* ';' ;
sassign: ID '=' rval  ;
rval: CONST | ID ;

type: 'int' ;

RETURN : 'return' ;
ID : [a-zA-Z_][a-zA-Z0-9_]* ;
CONST : '-'? [0-9]+ ;
COMMENT : '/*' .*? '*/' -> skip ;
COMMENT_LINE : '//' .*? '\n' -> skip ;
DIRECTIVE : '#' .*? '\n' -> skip ;
WS    : [ \t\r\n] -> channel(HIDDEN);
