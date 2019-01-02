parser grammar LvtParser;

options { tokenVocab = LvtLexer; }


lvt_file   : LVT lvt_version=float_
             LEVELNAME level_name=STR
             VERSION level_version=float_
             lvt_file_element*
             EOF
           ;

lvt_file_element
           : music
           | paralax
           | light_source
           | cmaps
           | palettes
           | shades
           | textures
           | sectors
           ;

float_ : INT | FLOAT ;

music        : MUSIC musicName=ID;
paralax      : PARALLAX float_ float_;
light_source : LIGHT SOURCE float_ float_ float_ float_;

shades    : SHADES numShades=INT (shade)* ;
shade     : SHADE COLON INT INT INT INT INT (L|G|T);

cmaps    : CMAPS numCMaps=INT (cmap)* ;
cmap     : CMAP_COLON cMapName=ID ;

palettes    : PALETTES numTextures=INT (palette)* ;
palette     : PALETTE_COLON paletteName=ID ;

textures    : TEXTURES numTextures=INT (texture)* ;
texture     : TEXTURE_COLON textureName=ID ;

vertices    : VERTICES numVertices=INT (vertex)* ;
vertex      : X COLON x=float_ Z COLON z=float_;

walls       : WALLS numWalls=INT (wall)* ;
wall        : WALL_COLON wallId=ID
              V1 COLON v1=INT
              V2 COLON v2=INT
              MID COLON mid=textureParamsSmall
              TOP COLON top=textureParamsSmall
              BOT COLON bot=textureParamsSmall
              OVERLAY COLON overlay=textureParamsSmall
              ADJOIN COLON adjoin=INT
              MIRROR COLON mirror=INT
              DADJOIN COLON dadjoin=INT
              DMIRROR COLON dmirror=INT
              FLAGS COLON flag0=INT flag1=INT
              LIGHT COLON light=INT
            ;

sectors     : NUMSECTORS numSectors=INT
              (sector)*;
textureParamsSmall : textureId=INT offsX=float_ offsY=float_;
textureParams : textureId=INT offsX=float_ offsY=float_ unused=float_;
sector      : SECTOR id=ID
              NAME (name=IDEND)?
              AMBIENT ambient=INT
              PALETTE paletteId=INT
              CMAP cmapId=INT
              FRICTION friction=float_
              GRAVITY gravity=INT
              ELASTICITY elasticity=float_
              VELOCITY velocityX=INT velocityY=INT velocityZ=INT
              VADJOIN vadjoin=INT
              FLOOR SOUND floorSound=ID
              (
                FLOOR TEXTURE floorTexture=textureParams
                FLOOR ALTITUDE altitude=float_
                |
                FLOOR Y  altitude=float_ floorTexture=textureParams
              )
              (
                CEILING TEXTURE ceilingTexture=textureParams
                CEILING ALTITUDE altitude=float_
                |
                CEILING Y  altitude=float_ ceilingTexture=textureParams
              )
              F_OVERLAY floorOverlayTexture=textureParams
              C_OVERLAY ceilingOverlayTexture=textureParams
              FLOOR OFFSETS floorOffsets=INT
              FLAGS flag0=INT flag1=INT (flag2=INT)?
              (SLOPEDFLOOR floorSlopeX=INT floorSlopeY=INT floorSlopeZ=INT)?
              (SLOPEDCEILING floorSlopeX=INT floorSlopeY=INT floorSlopeZ=INT)?
              LAYER INT
              vertices
              walls
            ;
