life: main.c
	cc -lsdl2 main.c -o life

play: life
	./life
