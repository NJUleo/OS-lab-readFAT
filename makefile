install:
	nasm -g -felf32 print.asm
	gcc -g -m32 readFAT.c print.o -o readFAT
nasm:
	nasm -g -felf32 print.asm
clean:
	rm *.o
	rm print
	rm readFAT
