SOURCES = src/pc_main.c $(wildcard src/gb-emu/*.c)
OBJECTS=$(SOURCES:.c=.o)
COMPILER_FLAGS = -c -O2 -std=c99
EXECUTABLE = pangb

ifeq ($(OS),Windows_NT)
CC = gcc
INCLUDE_PATHS = -IC:\mingw_dev_lib\include\SDL2
LIBRARY_PATHS = -LC:\mingw_dev_lib\lib
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2
else
CC = clang
INCLUDE_PATHS = -I/Library/Frameworks/SDL2.framework/Headers
LIBRARY_PATHS = -F/Library/Frameworks
LINKER_FLAGS = -framework SDL2
endif

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJECTS) -o $@
.c.o:
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@

clean: 
	@rm src/*.o
	@rm src/gb-emu/*.o