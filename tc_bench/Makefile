CFLAGS=-Os -Wall -Wextra -Wpedantic -Wno-unused-parameter

ALL = collatz-stc collatz-dtc-cps collatz-dtc-lav collatz-ttc

all: $(ALL)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ALL)

.PHONY: bench
bench: all
	$(foreach bench,$(ALL),echo;echo $(bench);time -f "\t%E" ./$(bench);)
