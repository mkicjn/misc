ALL = 2tos_rbpstack 2tos_prologue 1tos_rbpstack 1tos_prologue

all: $(ALL)

%: %.asm
	fasm $< $@
	chmod +x $@

.PHONY: clean
clean:
	rm -f $(ALL)
