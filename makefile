.PHONY: all
.PHONY: clean

all: win osx linux

.PHONY: win
windows:
	make -f Makefile.win

.PHONY: linux
linux:
	make -f Makefile.linux

.PHONY: osx
osx:
	make -f Makefile.osx

clean:
		@rm src/*.o
		@rm src/gb-emu/*.o
