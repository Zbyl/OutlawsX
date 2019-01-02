parser grammar LvtParser;

lvt_file   : LVT lvt_version=FLOAT
             LEVELNAME level_name=STR
             VERSION level_version=FLOAT
             lvt_file_elements
           ;

lvt_file_elements :
             (music)
             (paralax)
             (light_source)
             (cmaps)
             (palettes)
             (shades)
             (textures)
             (sectors)
           ;

music        : MUSIC musicName=ID;
paralax      : PARALLAX FLOAT FLOAT;
light_source : LIGHT SOURCE FLOAT FLOAT FLOAT FLOAT;

shades    : SHADES numShades=INT (shade)* ;
shade     : SHADE COLON INT INT INT INT INT UPPERCASE_LETTER;

cmaps    : CMAPS numCMaps=INT (cmap)* ;
cmap     : CMAP_COLON cMapName=ID ;

palettes    : PALETTES numTextures=INT (palette)* ;
palette     : PALETTE_COLON paletteName=ID ;

textures    : TEXTURES numTextures=INT (texture)* ;
texture     : TEXTURE_COLON textureName=ID ;

vertices    : VERTICES numVertices=INT (vertex)* ;
vertex      : X COLON x=FLOAT Y COLON y=FLOAT Z COLON z=FLOAT;

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
textureParamsSmall : textureId=INT offsX=FLOAT offsY=FLOAT;
textureParams : textureId=INT offsX=FLOAT offsY=FLOAT unused=INT;
sector      : SECTOR id=ID
              NAME name=ID
              AMBIENT ambient=INT
              PALETTE paletteId=INT
              CMAP cmapId=INT
              FRICTION friction=INT
              GRAVITY gravity=INT
              ELASTICITY elasticity=FLOAT
              VELOCITY velocityX=INT velocityY=INT velocityZ=INT
              VADJOIN vadjoin=INT
              FLOOR SOUND floorSound=ID
              (
                FLOOR TEXTURE floorTexture=textureParams
                FLOOR ALTITUDE altitude=INT
                |
                FLOOR Y  altitude=INT floorTexture=textureParams
              )
              (
                CEILING TEXTURE ceilingTexture=textureParams
                CEILING ALTITUDE altitude=INT
                |
                CEILING Y  altitude=INT ceilingTexture=textureParams
              )
              F_OVERLAY floorOverlayTexture=textureParams
              C_OVERLAY ceilingOverlayTexture=textureParams
              FLOOR OFFSETS floorOffsets=INT
              FLAGS flag0=INT flag1=INT (flag2=INT)?
              LAYER INT
              vertices
              walls
            ;
