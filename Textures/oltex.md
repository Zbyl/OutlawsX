# oltex

This document describes how to prepare texture atlas with all Outlaws textures.

# WARNING!!!
Textures that are not power of 2 need to be cropped (centered; and not resized, but cropped!) to be power of 2.
For example `GFPRCH2`, which is 64x66, need to have top and bottom line cut before creating the texture atlas.
This is necessary to get pixel perfect texturing and offsets.

# Process

1. Extract all `.pcx` files contained in `oltex.lab`.
   You can use `lab-fuse.exe` (project in this repository) to mount `.lab` file as a disk, and just copy out the files.
2. Convert all `.pcx` files to `.bmp`.
   See `batch-convert.bat` script for an idea how to convert all textures.  
   This script uses ImageMagick.  
3. Crop all the non-power of 2 textures (as mentioned above).
4. Install TexturePackerGUI (https://www.codeandweb.com/texturepacker).
4. Open `OutlawsX/Textures/oltex.tps` in TexturePackerGUI, which will compose all the textures into one huge texture atlas.
   Export the texture atlas to create atlas `OutlawsX/OutlawsXUnity/Assets/Textures/pack.png`, with meta data in `OutlawsX/OutlawsXUnity/Assets/Textures/pack.json`.

Resulting atlas `.png` file is imported by Unity (as a texture), `.json` file will be read by the `OutlawsPlugin.dll`.
