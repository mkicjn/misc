% Relational interpreter experiment

lisp_eval(_, t, t).

lisp_eval(_, [], []).

lisp_eval(_, [quote, X], X).

lisp_eval(Γ, [cons, A, B], [X|Y]) :-
	lisp_eval(Γ, A, X),
	lisp_eval(Γ, B, Y).

lisp_eval(Γ, [car, A], X) :-
	lisp_eval(Γ, A, [X|_]).

lisp_eval(Γ, [cdr, A], X) :-
	lisp_eval(Γ, A, [_|X]).

lisp_eval(Γ, [eq, A, B], t) :-
	lisp_eval(Γ, A, X),
	lisp_eval(Γ, B, X).

lisp_eval(_, [eq, _, _], []). % TODO: Can this be improved upon in pure Prolog?

lisp_eval(Γ, K, V) :-
	member(K=V, Γ).

lisp_eval(Γ, [[lambda, [A], B], C], R) :-
	lisp_eval(Γ, C, D),
	lisp_eval([A=D|Γ], B, R).

lisp_eval(Γ, [cond|Xs], R) :-
	member([If, Then], Xs),
	lisp_eval(Γ, If, t),
	lisp_eval(Γ, Then, R).

lisp_eval(Γ, [eval, X], R) :-
	lisp_eval(Γ, X, R).

% TODO: Iterative deepening for nested lists?
% miniKanren's interleaving strategy seems superior here
