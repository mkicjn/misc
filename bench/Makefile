ALL = \
	collatz_fasm \
	collatz_gcc \
	collatz_gforth-fast \
	collatz_gforth \
	collatz_gforth-itc \
	collatz_sbcl \
	collatz_pypy \
	collatz_python3 \
	collatz_tclsh \
	collatz_csc \
	collatz_csi \
	collatz_scheme \
	collatz_newlisp \
	collatz_lua \
	collatz_luajit \
	collatz_swipl

all: $(ALL)

.PHONY: bench
bench: all
	$(foreach bench,$(ALL),echo;echo $(bench);time -f "\t%E" ./$(bench);)

.PHONY: clean
clean:
	rm -f $(ALL)


collatz_fasm: src/collatz.asm
	fasm $^ $@
	chmod +x $@

collatz_gcc: src/collatz.c
	gcc -Ofast $^ -o $@

collatz_gforth-fast: src/collatz.fth
	echo "#! /usr/bin/gforth-fast" > $@
	cat $^ >> $@
	chmod +x $@
collatz_gforth: src/collatz.fth
	echo "#! /usr/bin/gforth" > $@
	cat $^ >> $@
	chmod +x $@
collatz_gforth-itc: src/collatz.fth
	echo "#! /usr/bin/gforth-itc" > $@
	cat $^ >> $@
	chmod +x $@

collatz_python3: src/collatz.py
	echo "#!/usr/bin/python3" > $@
	cat $^ >> $@
	chmod +x $@
collatz_pypy: src/collatz.py
	echo "#!/usr/bin/pypy" > $@
	cat $^ >> $@
	chmod +x $@

collatz_sbcl: src/collatz.lisp
	echo "#!/usr/bin/sbcl --script" > $@
	cat $^ >> $@
	chmod +x $@

collatz_csc: src/collatz.scm
	csc -O5 $^ -o $@
collatz_csi: src/collatz.scm
	echo "#!/usr/bin/csi -q" > $@
	cat $^ >> $@
	chmod +x $@
collatz_scheme: src/collatz.scm
	echo "#!/usr/bin/scheme -q" > $@
	cat $^ >> $@
	chmod +x $@
collatz_newlisp: src/collatz.lsp
	echo "#!/usr/bin/newlisp" > $@
	cat $^ >> $@
	chmod +x $@

collatz_tclsh: src/collatz.tcl
	echo "#!/usr/bin/tclsh" > $@
	cat $^ >> $@
	chmod +x $@


collatz_lua: src/collatz.lua
	echo "#!/usr/bin/lua" > $@
	cat $^ >> $@
	chmod +x $@

collatz_luajit: src/collatz.lua
	echo "#!/usr/bin/luajit" > $@
	cat $^ >> $@
	chmod +x $@

collatz_swipl: src/collatz.pl
	echo "#!/usr/bin/swipl" > $@
	cat $^ >> $@
	chmod +x $@
