CFLAGS=-Os -Wall -Wextra -Wpedantic

ALL = collatz-stc collatz-dtc collatz-ttc

all: $(ALL)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ALL)

.PHONY: bench
bench: all
	$(foreach bench,$(ALL),echo;echo $(bench);time -f "\t%E" ./$(bench);)
