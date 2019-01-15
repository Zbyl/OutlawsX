# OutlawsX

This is re-implementation of the LucasArts Outlaws and Dark Forces game engines.
Currently this is a very early prototype, with no gameplay functionality or rendering.

# License

MIT, with exception of third-party code (on their own licenses).

# Status

## Working

- Level geometry is correctly interpreted and generated.  
  Texture coordinates are mostly ok.
- Parser for LVT text level files (no LVB binary level files parser yet).
- Parser for INF script files.
- Parser for ATX animated texture files.
- Unity project for displaying geometry (see `OutlawsXUnity` subdirectory).

## Todo

Everything else.

# LabFuse

In LabFuse directory there is a working user-mode file system (requires Dokany installed) for mounting LAB package files as drives in Windows.  
It is written using FUSE, so it should be trivial to port to Linux.

# Used third-party lbraries

- `json` by Niels Lohmann (MIT license),
- `libtess` (`GLUTesselator`) by Silicon Graphics and AT&T (SGI FREE SOFTWARE LICENSE).
