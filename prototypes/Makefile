CFLAGS = -Os -g
WORD_LOCS = cps.c

cps: cps.c fthdef.h dict.h
	$(CC) $(CFLAGS) $< > $@

lav: lav.c fthdef.h dict.h
	$(CC) $(CFLAGS) $< > $@

dict.h: bind.pl $(WORD_LOCS)
	./$< $^ > $@

.PHONY: clean cleaner cleanest
clean:
	rm -f dict.h
cleaner: clean
	rm -f cps lav
cleanest: cleaner
	rm -f .*~ *~
