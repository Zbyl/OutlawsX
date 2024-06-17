lexer grammar AtxLexer;

ATX : 'ATX' ;
INST : 'INST' ;
RATE : 'RATE' ;
TEXTURE : 'TEXTURE' -> pushMode(idend_mode);
START_SOUND : 'START_SOUND' -> pushMode(idend_mode);
STOP : 'STOP' ;
GOTO : 'GOTO' ;

INT         : [+-]?[0-9]+ ;
FLOAT       : [+-]?[0-9]+ ( '.' [0-9]+ )? ;

WHITESPACE   : [ \t\r\n]+ -> channel(HIDDEN) ;
LINE_COMMENT : '#' ~[\r\n]* -> skip ;

mode idend_mode;
  ID          : [a-zA-Z0-9\-+_.~]+ ;
  WHITESPACE2 : [ \t]+ -> skip ;
  WHITESPACEEND   : [\r\n]+ -> skip, popMode ;
