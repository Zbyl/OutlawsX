# LAB Fuse

lab-fuse is a user-mode file system for LucasArts .LAB file format.

## Mount .LAB file as a drive

Usage:
```sh
lab-fuse <path-to-file.lab> <drive-letter> [fuse options]
```

The following command mounts `olobj.lab` file as drive `M:\`.

```sh
lab-fuse d:\outlaws\olobj.lab m
```

# Todo

1. Catch all exceptions in all functions.
2. Improve error reporting.