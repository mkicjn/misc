CFLAGS = -Os -g
WORD_LOCS = cps.c

a.out: cps.c fthdef.h dict.h
	$(CC) $(CFLAGS) $< -o $@

dict.h: bind.pl $(WORD_LOCS)
	./$< $^ > $@

.PHONY: clean cleaner cleanest
clean:
	rm -f dict.h
cleaner: clean
	rm -f a.out
cleanest: cleaner
	rm -f .*~ *~
