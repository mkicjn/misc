:- use_module(library(tabling)).

collatz(A, B) :-
	  0 is A mod 2, B is A // 2
	; 1 is A mod 2, B is 3 * A + 1.

:- table collatz_length/2.
collatz_length(1, 0) :- !.
collatz_length(N0, I0) :-
	collatz(N0, N),
	collatz_length(N, I),
	I0 is I + 1.

max_collatz_state(s(I0, Best0, Len0), s(I, Best, Len)) :-
	I is I0 + 1,
	collatz_length(I, L) ->
	  ( L >  Len0, !, Len = L, Best = I
	  ; L =< Len0, !, Len = Len0, Best = Best0 ).

reachable(_, S, S).
reachable(F, S0, S) :-
	call(F, S0, S1),
	reachable(F, S1, S).

max_collatz(Lim, N, M) :-
	reachable(max_collatz_state, s(0, 0, 0), s(Lim, N, M)).

:- initialization((max_collatz(1000000, _, M) -> write(M), halt)).
