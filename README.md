# pangb

pangb is an emulator for the Gameboy system. It's written in C, making it easily portable, and tries to emulate the original hardware as closely as possible.

## Status

At the moment pangb doesn't support:

- Sound
- All MBC chips except MBC1
- Link cable support
- STOP opcode
- Save RAM

## Build

pangb uses [SDL 2.0](http://www.libsdl.org). A makefile for Windows and OSX is provided but it should be possible to compile the project on Linux too.
