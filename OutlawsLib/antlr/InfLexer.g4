lexer grammar InfLexer;

INF : 'INF' ;
LEVELNAME_COLON : 'LEVELNAME:' -> pushMode(str_mode) ;
ITEMS : 'ITEMS' ;
ITEM_COLON : 'ITEM:' ;
SECTOR : 'SECTOR' ;
LINE : 'LINE' ;
LEVEL : 'LEVEL' ;
NAME_COLON : 'NAME:' -> pushMode(id_mode) ;
NUM_COLON : 'NUM:' -> pushMode(id_mode) ;
SEQ : 'SEQ' ;
SEQEND : 'SEQEND' ;
CLASS_COLON : 'CLASS:' ;
ELEVATOR : 'ELEVATOR' -> pushMode(id_mode) ;
TRIGGER : 'TRIGGER' -> pushMode(id_mode) ;
SOUND_COLON : 'SOUND:' -> pushMode(id_mode) ;
CENTER_COLON : 'CENTER:' ;
SPEED_COLON : 'SPEED:' ;
ANGLE_COLON : 'ANGLE:' ;
SLAVE_COLON : 'SLAVE:' -> pushMode(id_mode) ;
STOP_COLON : 'STOP:' ;
HOLD : 'HOLD' ;
EVENT_MASK_COLON : 'EVENT_MASK:' ;
OBJECT_COLON : 'OBJECT:' ;
OBJECT_EXCLUDE_COLON : 'OBJECT_EXCLUDE:' ;
AUTO : 'AUTO' ;
CLIENT_COLON : 'CLIENT:' -> pushMode(id_mode) ;
MESSAGE_COLON : 'MESSAGE:'-> pushMode(msg_mode)  ;

AT : '@' ;

INT         : [+-]?[0-9]+ ;
FLOAT       : [+-]?[0-9]+ ( '.' [0-9]+ )? ;

HACKY_0 : 'O' { setText( "0" ); } -> type(INT) ;  // This is a hack to pass error in original level HIDEOUT.LVT where letter O was used instead of a number 0.

WHITESPACE   : [ \t\r\n]+ -> channel(HIDDEN) ;
BLOCK_COMMENT : '/*' .*? '*/' -> skip ;

mode str_mode;
  STR: ~[\r\n]+ -> popMode;

mode id_mode;
  ID          : [a-zA-Z_.][a-zA-Z0-9\-+_.]* -> popMode ;
  INT2        : [+-]?[0-9]+ -> type(INT) ;
  WALLID2       : '#' [a-zA-Z0-9\-+_.]* -> type(WALLID) ;
  WHITESPACE2 : [ \t]+ -> skip ;
  WHITESPACEEND   : [\r\n]+ -> skip, popMode ;

mode recv_mode;
  ID5         : [a-zA-Z_.][a-zA-Z0-9\-+_.]* -> type(ID), popMode, pushMode(recv_mode2) ;
  INT5        : [+-]?[0-9]+ -> type(INT), popMode, pushMode(recv_mode2) ;
  SYSTEM      : 'SYSTEM' -> popMode, pushMode(recv_mode2) ;
  WHITESPACE6 : [ \t]+ -> skip ;
  WHITESPACEEND6   : [\r\n]+ -> skip, popMode ;

mode recv_mode2;
  WALLID       : '#' [a-zA-Z0-9\-+_.]* ;
  OPEN_PAREN  : '(' ;
  CLOSE_PAREN : ')' ;
  WHITESPACE5 : [ \t]+ -> skip ;
  WHITESPACEEND5   : [\r\n]+ -> skip, popMode ;

  // Messages
  M_TRIGGER5 : 'M_TRIGGER' -> type(M_TRIGGER), popMode ;
  M_TRIGGER_SMALL5 : 'm_trigger' -> type(M_TRIGGER), popMode ;
  GOTO_STOP5 : 'GOTO_STOP' -> type(GOTO_STOP), popMode ;
  GOTO_STOP_SMALL5 : 'goto_stop' -> type(GOTO_STOP), popMode ;
  NEXT_STOP5 : 'NEXT_STOP' -> type(NEXT_STOP), popMode ;
  NEXT_STOP_SMALL5 : 'next_stop' -> type(NEXT_STOP), popMode ;
  PREV_STOP5 : 'PREV_STOP' -> type(PREV_STOP), popMode ;
  PREV_STOP_SMALL5 : 'prev_stop' -> type(PREV_STOP), popMode ;
  MASTER_ON5 : 'MASTER_ON' -> type(MASTER_ON), popMode ;
  MASTER_ON_SMALL5 : 'master_on' -> type(MASTER_ON), popMode ;
  MASTER_OFF5 : 'MASTER_OFF' -> type(MASTER_OFF), popMode ;
  MASTER_OFF_SMALL5 : 'master_off' -> type(MASTER_OFF), popMode ;
  CLEAR_BITS5 : 'CLEAR_BITS' -> type(CLEAR_BITS), popMode ;
  CLEAR_BITS_SMALL5 : 'clear_bits' -> type(CLEAR_BITS), popMode ;
  SET_BITS5 : 'SET_BITS' -> type(SET_BITS), popMode ;
  SET_BITS_SMALL5 : 'set_bits' -> type(SET_BITS), popMode ;
  COMPLETE5 : 'COMPLETE' -> type(COMPLETE), popMode ;
  COMPLETE_SMALL5 : 'complete' -> type(COMPLETE), popMode ;
  DONE5 : 'DONE' -> type(DONE), popMode ;
  DONE_SMALL5 : 'done' -> type(DONE), popMode ;
  WAKEUP5 : 'WAKEUP' -> type(WAKEUP), popMode ;
  WAKEUP_SMALL5 : 'wakeup' -> type(WAKEUP), popMode ;
  LIGHTS5 : 'LIGHTS' -> type(LIGHTS), popMode ;
  LIGHTS_SMALL5 : 'lights' -> type(LIGHTS), popMode ;
  USER_MSG5 : 'USER_MSG' -> type(USER_MSG), popMode ;
  USER_MSG_SMALL5 : 'user_msg' -> type(USER_MSG), popMode ;

mode msg_mode;
  INT4        : [+-]?[0-9]+ -> type(INT), popMode, pushMode(recv_mode) ;
  WHITESPACE4 : [ \t]+ -> skip ;
  WHITESPACEEND4   : [\r\n]+ -> skip, popMode ;

  // Messages
  M_TRIGGER : 'M_TRIGGER' -> popMode ;
  M_TRIGGER_SMALL : 'm_trigger' -> popMode ;
  GOTO_STOP : 'GOTO_STOP' -> popMode ;
  GOTO_STOP_SMALL : 'goto_stop' -> popMode ;
  NEXT_STOP : 'NEXT_STOP' -> popMode ;
  NEXT_STOP_SMALL : 'next_stop' -> popMode ;
  PREV_STOP : 'PREV_STOP' -> popMode ;
  PREV_STOP_SMALL : 'prev_stop' -> popMode ;
  MASTER_ON : 'MASTER_ON' -> popMode ;
  MASTER_ON_SMALL : 'master_on' -> popMode ;
  MASTER_OFF : 'MASTER_OFF' -> popMode ;
  MASTER_OFF_SMALL : 'master_off' -> popMode ;
  CLEAR_BITS : 'CLEAR_BITS' -> popMode ;
  CLEAR_BITS_SMALL : 'clear_bits' -> popMode ;
  SET_BITS : 'SET_BITS' -> popMode ;
  SET_BITS_SMALL : 'set_bits' -> popMode ;
  COMPLETE : 'COMPLETE' -> popMode ;
  COMPLETE_SMALL : 'complete' -> popMode ;
  DONE : 'DONE' -> popMode ;
  DONE_SMALL : 'done' -> popMode ;
  WAKEUP : 'WAKEUP' -> popMode ;
  WAKEUP_SMALL : 'wakeup' -> popMode ;
  LIGHTS : 'LIGHTS' -> popMode ;
  LIGHTS_SMALL : 'lights' -> popMode ;
  USER_MSG : 'USER_MSG' -> popMode ;
  USER_MSG_SMALL : 'user_msg' -> popMode ;
