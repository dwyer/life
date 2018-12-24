BIN=life.exe

play: $(BIN)
	./$<

$(BIN): main.c
	$(CC) -lsdl2 -o $@ $+

clean:
	$(RM) *.exe *.o
