# Experimental Lisp interpreter

The interpreter `lisp2.c` is a slightly modified version of `lisp-small.c` from [c2kanren](https://github.com/mkicjn/c2kanren) (another one of my projects).

The rcfile `rc.lisp` has been rewritten mostly from scratch, although the approach is essentially the same.

Right now, the only *major* difference is that `if` is the primitive conditional form used instead of `cond`.
Macro `cond` in `rc.lisp` translate `cond` expressions back to `if`, such that either one is still usable practice.

I did this because it might be fun to try to implement J-Bob from "The Little Prover," by Daniel P. Friedman and Carl Eastlund,
but I figured not supporting `if` natively might make it a little awkward to translate.
So, that's what this experimental interpreter might be used for eventually.

