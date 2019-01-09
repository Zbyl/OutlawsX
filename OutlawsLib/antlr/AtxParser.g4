parser grammar AtxParser;

options { tokenVocab = AtxLexer; }


atx_file   : ATX atx_version=float_
             INST numInstructions=INT
             atx_instruction*
             EOF
           ;

atx_instruction
           : rate           // This is only ever used as the first instruction.
           | texture
           | start_sound
           | stop
           | goto_
           | goto_stop
           ;

float_ : INT | FLOAT ;

rate        : RATE rateValue=INT ;
texture     : TEXTURE textureName=ID ;
start_sound : START_SOUND unusedInt=ID soundName=ID ;
stop        : STOP ;
goto_       : GOTO instructionIdx=INT ;
goto_stop   : GOTO STOP unknown=INT ; // This is not actually used in game.
