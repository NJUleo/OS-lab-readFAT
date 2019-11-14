install:
	nasm -g -felf32 print.asm
	g++ -g -m32 readFAT.cpp print.o -o readFAT
nasm:
	nasm -g -felf32 print.asm
clean:
	rm *.o
	rm print
	rm readFAT
