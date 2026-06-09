% (WIP) Type-directed (?) relational interpreter experiment
% For simply-typed λ-calculus with Bool and Nat
%
% TODO: Does this have the intended advantage, or is it ultimately limited by iterative deepening, same as the other?

lc_type(Term, Type) :-
	lc_etype([], Term, Type).

:- discontiguous lc_etype/3.


lc_eval(Term, Type, Val) :-
	lc_eeval([], Term, Type, Val).

:- discontiguous lc_eeval/4.


lc_term(T) :-
	length(S, _),
	phrase(lc_serial(T), S).

:- discontiguous lc_serial//1.


% Ground terms
lc_sertype(bool) --> [bool].
lc_sertype(nat) --> [nat].
lc_sertype(A->B) --> ['->'], lc_sertype(A), lc_sertype(B).

lc_serial(true) --> [true].
lc_etype(_, true, bool).
lc_eeval(_, true, bool, true).

lc_serial(false) --> [false].
lc_etype(_, false, bool).
lc_eeval(_, false, bool, false).

lc_serial(0) --> [0].
lc_etype(_, 0, nat).
lc_eeval(_, 0, nat, 0).

% Bool operators
lc_not(true, false).
lc_not(false, true).

lc_serial(not(X)) --> [not], lc_serial(X).
lc_etype(Γ, not(X), bool) :-
	lc_etype(Γ, X, bool).
lc_eeval(Env, not(X), bool, Y) :-
	lc_eeval(Env, X, bool, X2),
	lc_not(X2, Y).

lc_if(Env, if(true,Then,_), T, X) :-
	lc_eeval(Env, Then, T, X).
lc_if(Env, if(false,_,Else), T, X) :-
	lc_eeval(Env, Else, T, X).

lc_serial(if(Cond,Then,Else)) --> [if], lc_serial(Cond), lc_serial(Then), lc_serial(Else).
lc_etype(Γ, if(Cond,Then,Else), T) :-
	lc_etype(Γ, Cond, bool),
	lc_etype(Γ, Then, T),
	lc_etype(Γ, Else, T).
lc_eeval(Env, if(Cond,Then,Else), T, X) :-
	lc_eeval(Env, Cond, bool, Bool),
	lc_if(Env, if(Bool,Then,Else), T, X).

% Nat operators
lc_serial(succ(X)) --> [succ], lc_serial(X).
lc_etype(Γ, succ(X), nat) :-
	lc_etype(Γ, X, nat).
lc_eeval(Env, succ(X), nat, succ(Y)) :-
	lc_eeval(Env, X, nat, Y).

lc_iszero(0, true).
lc_iszero(succ(_), false).

lc_serial(iszero(X)) --> [iszero], lc_serial(X).
lc_etype(Γ, iszero(X), bool) :-
	lc_etype(Γ, X, nat).
lc_eeval(Env, iszero(X), bool, Y) :-
	lc_eeval(Env, X, nat, X2),
	lc_iszero(X2, Y).

lc_pred(0, 0).
lc_pred(succ(X), X).

lc_serial(pred(X)) --> [pred], lc_serial(X).
lc_etype(Γ, pred(X), nat) :-
	lc_etype(Γ, X, nat).
lc_eeval(Env, pred(X), nat, Y) :-
	lc_eeval(Env, X, nat, X2),
	lc_pred(X2, Y).

% Functions and variables
lc_serial(lambda(Arg:ArgType, Body)) --> [lambda, Arg], lc_sertype(ArgType), lc_serial(Body).
lc_etype(Γ, lambda(Arg:ArgType, Body), (ArgType->ResType)) :-
	lc_etype([Arg:ArgType-_|Γ], Body, ResType).
lc_eeval(Env, lambda(Arg:ArgType, Body), (ArgType->ResType), closure(Env, Arg:ArgType, Body)) :-
	lc_etype([Arg:ArgType-_|Env], Body, ResType).

lc_serial(var(X)) --> [var, X].
lc_etype(Γ, var(V), VType) :-
	member(V:VType-_, Γ).
lc_eeval(Env, var(V), VType, VVal) :-
	member(V:VType-VVal, Env).

lc_apply(closure(Env, Arg:ArgType, Body), ArgVal, ResType, Res) :-
	lc_eeval([Arg:ArgType-ArgVal|Env], Body, ResType, Res).

lc_serial(apply(Func, Arg)) --> [apply], lc_serial(Func), lc_serial(Arg).
lc_etype(Γ, apply(Func, Arg), ResType) :-
	lc_etype(Γ, Arg, ArgType),
	lc_etype(Γ, Func, (ArgType->ResType)).
lc_eeval(Env, apply(Func, Arg), ResType, Res) :-
	lc_eeval(Env, Arg, ArgType, ArgVal),
	lc_eeval(Env, Func, (ArgType->ResType), Closure),
	lc_apply(Closure, ArgVal, ResType, Res).
