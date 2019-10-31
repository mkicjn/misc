CFLAGS=-Os -g
DICT_LOCS=cps2.c

a.out: cps2.c fthdef.h dict.h
	$(CC) $(CFLAGS) $<

dict.h: bind.pl $(DICT_LOCS)
	./$< $^ > $@

.PHONY: clean cleaner cleanest
clean:
	rm -f dict.h
cleaner: clean
	rm -f a.out
cleanest: cleaner
	rm -f .*~ *~
