# panz-gb

panz-gb is an emulator for the Gameboy system. It's written in C and tries to emulate the original hardware as closely as possible.

## Status

At the moment panz-gb doesn't support:

- Sound
- All MBC chips except MBC1 and (partially) MBC3
- Link cable support
- STOP opcode
- Save RAM

## Build

The panz-gb frontend uses [SDL 2.0](http://www.libsdl.org). A makefile is provided for Linux but you should be able to compile the project with any C99 compatible compiler.
