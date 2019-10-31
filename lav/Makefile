CFLAGS = -Os -g
WORD_LOCS = main.c

a.out: main.c fthdef.h dict.c
	$(CC) $(CFLAGS) $< -o $@

dict.c: bind.pl $(WORD_LOCS)
	./$< $(WORD_LOCS) > $@

.PHONY: clean cleaner cleanest
clean:
	rm -f dict.c
cleaner: clean
	rm -f a.out
cleanest: cleaner
	rm -f .*~ *~
