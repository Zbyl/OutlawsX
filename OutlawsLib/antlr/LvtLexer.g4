lexer grammar LvtLexer;

ALTITUDE : 'ALTITUDE' ;
AMBIENT : 'AMBIENT' ;
C_OVERLAY : 'C_OVERLAY' ;
CEILING : 'CEILING' ;
CMAP : 'CMAP' ;
CMAPS : 'CMAPS' ;
COLON : ':' ;
ELASTICITY : 'ELASTICITY' ;
F_OVERLAY : 'F_OVERLAY' ;
FLAGS : 'FLAGS' ;
FLOOR : 'FLOOR' ;
FRICTION : 'FRICTION' ;
GRAVITY : 'GRAVITY' ;
LAYER : 'LAYER' ;
LIGHT : 'LIGHT' ;
LVT : 'LVT' ;
NUMSECTORS : 'NUMSECTORS' ;
OFFSETS : 'OFFSETS' ;
PALETTE : 'PALETTE' ;
PALETTES : 'PALETTES' ;
PARALLAX : 'PARALLAX' ;
SHADE : 'SHADE' ;
SHADES : 'SHADES' ;
SOURCE : 'SOURCE' ;
TEXTURE : 'TEXTURE' ;
TEXTURES : 'TEXTURES' ;
VADJOIN : 'VADJOIN' ;
VELOCITY : 'VELOCITY' ;
VERSION : 'VERSION' ;
X : 'X' ;
Y : 'Y' ;
Z : 'Z' ;
L : 'L' ;
G : 'G' ;
T : 'T' ;
VERTICES : 'VERTICES' ;
WALLS : 'WALLS' ;
V1 : 'V1' ;
V2 : 'V2' ;
MID : 'MID' ;
TOP : 'TOP' ;
BOT : 'BOT' ;
OVERLAY : 'OVERLAY' ;
ADJOIN: 'ADJOIN' ;
MIRROR : 'MIRROR' ;
DADJOIN: 'DADJOIN' ;
DMIRROR : 'DMIRROR' ;
SLOPEDFLOOR : 'SLOPEDFLOOR' ;
SLOPEDCEILING : 'SLOPEDCEILING' ;

LEVELNAME   : 'LEVELNAME' -> pushMode(str_mode);
SECTOR      : 'SECTOR' -> pushMode(id_mode);
NAME        : 'NAME' -> pushMode(idend_mode);
SOUND       : 'SOUND' -> pushMode(id_mode);
MUSIC       : 'MUSIC' -> pushMode(id_mode);
WALL_COLON    : 'WALL' ':' -> pushMode(id_mode);
TEXTURE_COLON : 'TEXTURE' ':' -> pushMode(id_mode);
PALETTE_COLON : 'PALETTE' ':' -> pushMode(id_mode);
CMAP_COLON    : 'CMAP' ':' -> pushMode(id_mode);

INT         : [+-]?[0-9]+ ;
FLOAT       : [+-]?[0-9]+ ( '.' [0-9]+ )? ;

WHITESPACE   : [ \t\r\n]+ -> channel(HIDDEN) ;
LINE_COMMENT : '#' ~[\r\n]* -> skip ;

mode str_mode;
  STR: ~[\r\n]+ -> popMode;

mode id_mode;
  ID: [a-zA-Z0-9\-_.]+ -> popMode;
  WHITESPACE2   : [ \t\r\n]+ -> channel(HIDDEN) ;

mode idend_mode;
  IDEND: [a-zA-Z0-9\-_.]+;
  WHITESPACE3   : [ \t]+ -> channel(HIDDEN) ;
  WHITESPACEEND   : [\r\n]+ -> skip, popMode ;
