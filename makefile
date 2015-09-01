OBJS = pc_main.c
OBJS_EMU = gb-emu/gb-memory.c gb-emu/gc-imp.c gb-emu/gb-video.c gb-emu/gb-opcodes.c gb-emu/gc-interrupts.c gb-emu/gb-opcodes-impl.c

ifeq ($(OS),Windows_NT)
CC = gcc
INCLUDE_PATHS = -IC:\mingw_dev_lib\include\SDL2
LIBRARY_PATHS = -LC:\mingw_dev_lib\lib
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2
else
CC = clang
INCLUDE_PATHS =
LIBRARY_PATHS =
LINKER_FLAGS = -framework SDL2
endif

COMPILER_FLAGS = -O2 -std=c99
OBJ_NAME = pangb

all : $(OBJS)
	$(CC) $(OBJS) $(OBJS_EMU) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
