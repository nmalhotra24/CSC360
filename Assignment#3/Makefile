.phony all:

all: Assignment3

Assignment3:	diskinfo.c disklist.c diskget.c 
	gcc diskinfo.c -o diskinfo
	gcc disklist.c -o disklist
	gcc diskget.c -o diskget
.PHONY clean:
clean:
	-rm -rf diskinfo
	-rm -rf disklist
	-rm -rf diskget
