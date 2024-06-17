# OutlawsX

This is re-implementation of the LucasArts Outlaws and Dark Forces game engines.
Currently this is a very early prototype, with no gameplay functionality.

# License

MIT, with exception of third-party code (on their own licenses).

# Status

## Working

- Level geometry is correctly interpreted and generated, including slopes.  
- Texture coordinates are correct (as far as I could see), including floor/ceiling rotations and offsets.
- Parser for LVT text level files.
- Code for converting LVB binary level files into LVT files.
- Parser for INF script files.
- Parser for ATX animated texture files.
- Unity project for displaying geometry (see `OutlawsXUnity` subdirectory).
- `lab-fuse` - utility for mouting `.lab` files as Windows disks.

## Todo

- ATX textures are not displayed. ATX files are not processed.
- There is no game logic.
- There are no objects, enemies, weapons, cuscenes, menus, etc.
- INF files are not processed.
- There is no music, sounds.
- There is no water.
- There is no shading, lights, tinting.
- There is no sky/pit.
- Everything else...

# How to try it out in Unity

1. Extract a level file (`.lvt` or `.lvb`) from Outlaws `olgeo.lab` (I suggest `HIDEOUT` or `CANYON`, as those are tested).  
   You can use LabFuse for that (available in Releases on GitHub).  
   See LabFuse docs below for information about how to run it.
2. Copy `OutlawsPlugin.dll` and `antlr4-runtime.dll` to `OutlawsXUnity` directory.  
   You can either use the released versions (see Releases on GitHub), or compile it yourself (as explained in sections below).
3. Open `OutlawsXUnity` in Unity (tested on Unity version `2022.3.33f1`).
4. In `GameHandler` object set `Level File` to your `.lvt` or `.lvb` file.
5. In `GameHandler` object update path `Textures Pack File` to the `OutlawsX\OutlawsXUnity\Assets\Textures\pack.json` file.
6. Play.
7. Fly around using WSAD + mouse or game pad.

# Compilation

1. You might need to install Visual Studio Community 2022 to get the necessary compilers and tools.
2. Install CMake: https://cmake.org/download/
3. Install Vcpkg:
   ```
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg && bootstrap-vcpkg.bat
   ```
4. Clone `OutlawsX`.
5. Clone [ArbitraryFormatSerializer](https://github.com/Zbyl/ArbitraryFormatSerializer) next to `OutlawsX` directory.
6. Optionally install Dokan 2.1 (needed for LabFuse project): https://github.com/dokan-dev/dokany/releases
7. In steps below we assume that `vpckg` was installed in `<some-dir>/vcpkg`, and `OutlawsX` was cloned to `<some-dir>/OutlawsX`
8. Download ANTLR4 Jar file: https://www.antlr.org/download/antlr-4.13.1-complete.jar
9. Install CMake.
10. Use CMake to build `OutlawsPlugin` for Unity and optionally `LabFuse`.
11. Generate project:
   ```
   cmake -DCMAKE_TOOLCHAIN_FILE=<some-dir>/vcpkg/scripts/buildsystems/vcpkg.cmake ^
         -DVCPKG_TARGET_TRIPLET=x64-windows ^
         -DCMAKE_INSTALL_PREFIX="<some-dir>\OutlawsX\OutlawsXUnity" ^
         -DDOKAN_HOME="C:\Program Files\Dokan\Dokan Library-2.1.0" ^
         -DANTLR4_JAR_LOCATION="<some-dir>\antlr-4.13.1-complete.jar" ^
         -S <some-dir>/OutlawsX 6
         -B <some-dir>/OutlawsX-build
   ```
   > **Note:** `DOKAN_HOME` is needed only if LabFuse is to be built.

   > **Note:** Compilation make take a long time, due to building `boost` libraries.
12. Build the project:
   ```
   cmake --build <some-dir>/OutlawsX-build --config Release
   ```
13. Copy `OutlawsPlugin.dll` to `OutlawsXUnity`:
   ```
   cmake --install <some-dir>/OutlawsX-build --config Release
   ```
14. Don't forget to copy `antlr4-runtime.dll` to `OutlawsXUnity` directory as well.  
   You will find it in `<some-dir>/OutlawsX-build/OutlawsPlugin/Release`.

> **Note:** You can generate VisualStudio project using: `cmake -G "Visual Studio 17 2022" -A x64 ...`. Open Visual Studio project (`<some-dir>/OutlawsX-build/OutlawsX.sln`) and build it. Then build `INSTALL` target.

> **Note:** You can use `INSTALL` target in VisualStudio or `cmake --install <build-dir> --config Release` to copy `OutlawsPlugin.dll` to `OutlawsXUnity` directory.


# Generating texture packs

This plugin uses textures extracted from Outlaws and merged into one texture atlas.

If you'd like to re-generate the texture atlas see documentation here: [oltex/README.md](./oltex/README.md)


# LabFuse

In LabFuse directory there is a working user-mode file system (requires Dokany installed) for mounting LAB package files as drives in Windows.  
It is written using FUSE, so it should be trivial to port to Linux.

See documentation here: [LabFuse/README.md](./LabFuse/README.md)

# Outlaws specs and file formats

See here: [OutlawsHlpFiles](./OutlawsHlpFiles)

# Used third-party lbraries

- `json` by Niels Lohmann (MIT license),
- `libtess` (`GLUTesselator`) by Silicon Graphics and AT&T (SGI FREE SOFTWARE LICENSE).
