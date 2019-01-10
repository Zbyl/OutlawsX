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

classElement  : sound_
              | center_
              | speed_
              | angle_
              | slave_
              | stop_
              | master_
              | eventMask_
              | event_
              | entityMask_
              | start_
              | key_
              | flags_
              | objectMask_
              | object_
              | objectExclude_
              | client_
              | triggerMessage
              | elevatorMessage
              ;

sound_          : SOUND_COLON soundEvent=INT soundName=ID ;
center_         : CENTER_COLON centerX=float_ centerZ=float_ ;
speed_          : SPEED_COLON speed=float_ ;
angle_          : ANGLE_COLON angle=float_ ;
slave_          : SLAVE_COLON slaveName=ID ;
stop_           : STOP_COLON stopValue=float_ (stopTime=INT | stopKind=(HOLD | TERMINATE | COMPLETE) )?
                | STOP_COLON AT stopValue=float_ (stopTime=INT | stopKind=(HOLD | TERMINATE | COMPLETE) )?
                | STOP_COLON sectorName=ID stopValue=float_ (hold=HOLD | stopTime=INT)?
                ;
master_         : MASTER_COLON masterSwitch=(ON | OFF) ;
eventMask_      : EVENT_MASK_COLON eventMask=(INT | STAR) ;
event_          : EVENT_COLON event=INT ;
entityMask_     : ENTITY_MASK_COLON entityMask=(INT | STAR) ;
start_          : START_COLON start=INT ;
key_            : KEY_COLON key=(RED | BLUE | YELLOW) ;
flags_          : FLAGS_COLON flags=INT ;
objectMask_     : OBJECT_MASK_COLON objectMask=INT ;
object_         : OBJECT_COLON obj0=INT obj1=INT AUTO? ;
objectExclude_  : OBJECT_EXCLUDE_COLON obj0=INT AUTO? ;
client_         : CLIENT_COLON client=clientId ;
triggerMessage  : MESSAGE_COLON message ;
elevatorMessage : MESSAGE_COLON stopIdx=INT client=clientId message ;

clientId      : ID
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
