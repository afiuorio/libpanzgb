SOURCES = src/pc_main.c $(wildcard src/gb-emu/*.c)
OBJECTS=$(SOURCES:.c=.o)
COMPILER_FLAGS = -c -O2 -std=c99 -Wall -Wextra -pedantic -flto -march=native
EXECUTABLE = panz-gb

CC = gcc
INCLUDE_PATHS =
LIBRARY_PATHS =
LINKER_FLAGS = -lSDL2

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBRARY_PATHS) $(LINKER_FLAGS) -o $@
.c.o:
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@

clean:
	@rm src/*.o src/gb-emu/*.o
