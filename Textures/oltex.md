# oltex

In this derectory there should be `.bmp` files converted from all `.pcx` files contained in `oltex.lab`.  
Those files will then be composed into one big texture atlas using TexturePackerGUI project: `OutlawsX/Textures/oltex.tps'.  
Resulting atlas will be put in `OutlawsX/OutlawsXUnity/Assets/Textures/pack.png`, with meta data in `OutlawsX/OutlawsXUnity/Assets/Textures/pack.json`.  
`.png` file is read by Unity, `.json` file will be read by the `OutlawsPlugin.dll`.

See `batch-convert.bat` script for an idea how to convert all textures.  
This script uses ImageMagick.  
*Note:* You can use LabFuse (in this repo) to mount `.lab` file as a Windows drive.
