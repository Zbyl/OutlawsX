parser grammar InfParser;

options { tokenVocab = InfLexer; }


inf_file   : INF inf_version=float_
             LEVELNAME_COLON levelName=STR 
             ITEMS numItems=INT
             item*
             EOF
           ;

item       : ITEM_COLON itemType=(LEVEL | SECTOR | LINE)
             NAME_COLON itemName=ID
             (NUM_COLON wallName=WALLID)?
             SEQ class_* SEQEND
           ;

class_     : CLASS_COLON classType=(ELEVATOR | TRIGGER) className=ID
             classElement*
           ;

classElement  : SOUND_COLON soundEvent=INT soundName=ID
              | CENTER_COLON centerX=float_ centerZ=float_
              | SPEED_COLON speed=float_
              | ANGLE_COLON angle=float_
              | SLAVE_COLON slaveName=ID
              | STOP_COLON stopValue=float_ (hold=HOLD | stopTime=INT)?
              | STOP_COLON AT stopValue=float_ (hold=HOLD | stopTime=INT)?
              | STOP_COLON sectorName=ID stopValue=float_ (hold=HOLD | stopTime=INT)?
              | EVENT_MASK_COLON eventMask=INT
              | OBJECT_COLON obj0=INT obj1=INT AUTO?
              | OBJECT_EXCLUDE_COLON obj0=INT AUTO?
              | CLIENT_COLON client=client_
              | MESSAGE_COLON message
              | MESSAGE_COLON stopIdx=INT client=client_ message
              ;

client_       : ID
              | ID OPEN_PAREN wallName=WALLID CLOSE_PAREN
              | SYSTEM
              ;

message       : m_trigger
              | goto_stop
              | next_stop
              | prev_stop
              | master_on
              | master_off
              | clear_bits
              | set_bits
              | complete
              | done
              | wakeup
              | lights
              | user_msg
              ;

m_trigger     : messageName=(M_TRIGGER | M_TRIGGER_SMALL) (eventValue=INT)? ;
goto_stop     : messageName=(GOTO_STOP | GOTO_STOP_SMALL) stopIdx=INT ;
next_stop     : messageName=(NEXT_STOP | NEXT_STOP_SMALL) (eventValue=INT)? ;
prev_stop     : messageName=(PREV_STOP | PREV_STOP_SMALL) (eventValue=INT)? ;
master_on     : messageName=(MASTER_ON | MASTER_ON_SMALL) (eventValue=INT)? ;
master_off    : messageName=(MASTER_OFF | MASTER_OFF_SMALL) (eventValue=INT)? ;
clear_bits    : messageName=(CLEAR_BITS | CLEAR_BITS_SMALL) flagNumber=INT bitNumber=INT ;
set_bits      : messageName=(SET_BITS | SET_BITS_SMALL) flagNumber=INT bitNumber=INT ;
complete      : messageName=(COMPLETE | COMPLETE_SMALL) goalNumber=INT ;
done          : messageName=(DONE | DONE_SMALL) ;
wakeup        : messageName=(WAKEUP | WAKEUP_SMALL) ;
lights        : messageName=(LIGHTS | LIGHTS_SMALL) ;
user_msg      : messageName=(USER_MSG | USER_MSG_SMALL) msgNumber=INT ;

float_ : INT | FLOAT ;
