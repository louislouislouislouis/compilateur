grammar ifcc;

axiom: prog;

prog: 'int' 'main' '(' ')' '{' expr* '}';

expr: (ret | decl | assign | inlineArithmetic)? ';'
	| conditionnal
	| loopW
	| loopCtrl
	| '{' expr* '}';

ret: 'return' rval;

decl: type sdecl (',' sdecl)*;
sdecl: ID ('=' rval)?;

assign: sassign (',' sassign)*;

sassign:
	ID OPASSIGN = (
		'='
		| '+='
		| '-='
		| '*='
		| '/='
		| '%='
		| '&='
		| '|='
		| '^='
		| '<<='
		| '>>='
	) rval;

rval: arithmetic;

type: 'int' | 'char';

inlineArithmetic: arithmetic;
arithmetic:
	op = ('-' | '+' | '!' | '~') arithmetic							# unary
	| arithmetic op = ('*' | '/' | '%') arithmetic					# muldiv
	| arithmetic op = ('+' | '-') arithmetic						# addminus
	| arithmetic op = ('<=' | '>=' | '<' | '>') arithmetic			# comprel
	| arithmetic op = ('==' | '!=') arithmetic						# compeq
	| arithmetic op = ('<<' | '>>') arithmetic						# shift
	| arithmetic op = ('&&' | '||') arithmetic						# oplog
	| arithmetic op = ('&' | '^' | '|' | '<<' | '>>') arithmetic	# bitwise
	| '(' arithmetic ')'											# par
	| ID OPASSIGN = (
		'='
		| '+='
		| '-='
		| '*='
		| '/='
		| '%='
		| '&='
		| '|='
		| '^='
		| '<<='
		| '>>='
	) arithmetic	# assignChain
	| CONST			# const
	| ID			# id;

conditionnal:
	'if' '(' inlineArithmetic ')' ifexpr ('else' elseexpr)?;

ifexpr: expr | '{' expr+ '}';
elseexpr: conditionnal | expr | '{' expr+ '}';

loopW:
	'while' '(' inlineArithmetic ')' (expr | '{' (expr)+ '}');
loopCtrl: keyW = ('break' | 'continue') ';';

OPTOK: '--' | '++';
RETURN: 'return';
ID: [a-zA-Z_][a-zA-Z0-9_]*;
CONST: [0-9]+ | '\'' .'\'';
COMMENT: '/*' .*? '*/' -> skip;
COMMENT_LINE: '//' .*? '\n' -> skip;
DIRECTIVE: '#' .*? '\n' -> skip;
WS: [ \t\r\n] -> channel(HIDDEN);
