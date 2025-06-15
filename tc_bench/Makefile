CFLAGS=-Os -Wall -Wextra -Wpedantic -Wno-unused-parameter

ALL = collatz-stc collatz-dtc-cps collatz-dtc-lav collatz-ttc

all: $(ALL)

%.o: ../%.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ALL)

.PHONY: bench
bench: all
	$(foreach bench,$(ALL),echo;echo $(bench);time -f "\t%E" ./$(bench);)

.PHONY: analysis
analysis: fth_dict.o
	$(CROSS_COMPILE)objdump -d $< | ./analyze.py
