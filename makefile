.PHONY: all
.PHONY: clean

all: win osx

.PHONY: win
windows:
	make -f Makefile.win

.PHONY: osx
osx:
	make -f Makefile.osx

clean:
		@rm src/*.o
		@rm src/gb-emu/*.o
