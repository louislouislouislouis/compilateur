grammar ifcc;

axiom: prog;

prog: 'int' 'main' '(' ')' '{' expr+ '}';

expr: (ret | decl | assign | inlineArithmetic)? ';';

ret: 'return' rval;

decl: type sdecl (',' sdecl)*;
sdecl: ID ('=' rval)?;

assign: sassign (',' sassign)*;
sassign: ID '=' rval;
rval: ID | arithmetic;

type: 'int';

inlineArithmetic: arithmetic;
arithmetic:
	op = ('-' | '+' | '!') arithmetic						# unary
	| arithmetic op = ('*' | '/' | '%') arithmetic			# muldiv
	| arithmetic op = ('+' | '-') arithmetic				# addminus
	| arithmetic op = ('<=' | '>=' | '<' | '>') arithmetic	# comprel
	| arithmetic op = ('==' | '!=') arithmetic				# compeq
	| arithmetic op = ('&&' | '||') arithmetic				# oplog
	| '(' arithmetic ')'									# par
	| CONST													# const
	| ID													# id;

RETURN: 'return';
ID: [a-zA-Z_][a-zA-Z0-9_]*;
CONST: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
COMMENT_LINE: '//' .*? '\n' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
