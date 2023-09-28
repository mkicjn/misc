ALL = baseline 2tos_rbpstack 2tos_prologue 2tos_xchgspbp 1tos_rbpstack 1tos_prologue 1tos_xchgspbp

all: $(ALL)

%: %.asm
	fasm $< $@
	chmod +x $@

.PHONY: clean
clean:
	rm -f $(ALL)

.PHONY: bench
bench: all
	$(foreach bench,$(ALL),echo;echo $(bench);time -f "\t%E" ./$(bench);)
