# Experimental Lisp interpreter

The interpreter `c2klisp.c` is a modified version of `lisp-small.c` from [c2kanren](https://github.com/mkicjn/c2kanren) (another one of my projects).
Neither of those interpreters support numeric values; for that, see `c2klispn.c`, which has been lightly extended to support numbers.

The objective of adding numeric support with a minimal diff was challenging, but has perhaps led to a better result than either of the original interpreters.

The rcfile `rc.lisp` has been rewritten mostly from scratch, although the approach is essentially the same.

Right now, the only *major* difference is that `if` is the primitive conditional used instead of `cond`.
Macro `cond` in `rc.lisp` translates `cond` expressions back to `if`, such that either one is still usable in practice.

Originally, I did this because it might be fun to try to implement J-Bob from "The Little Prover," by Daniel P. Friedman and Carl Eastlund,
but I figured not supporting `if` natively might make it a little awkward to translate.
So, this experimental interpreter might be used for that eventually, or perhaps other related (mini?)projects.

There are some other miscellaneous changes (such as completely removing fexprs), but there's no particular reason behind those.
Ultimately, this is just here to preserve any experimental changes that aren't ready to push to c2kanren.
