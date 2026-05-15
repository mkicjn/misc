% Relational interpreter experiment
% (For simply-typed λ-calculus with Bool and Nat)
% (Also, with type inference)

:- use_module(library(lists)).
:- use_module(library(clpfd)).

% Typing rules

lc_env_type(_, true, bool).
lc_env_type(_, false, bool).

lc_env_type(Env, not(X), bool) :-
	lc_env_type(Env, X, bool).

lc_env_type(Env, if(A,B,C), T) :-
	lc_env_type(Env, A, bool),
	lc_env_type(Env, B, T),
	lc_env_type(Env, C, T).

lc_env_type(_, 0, nat).
lc_env_type(Env, succ(N), nat) :-
	lc_env_type(Env, N, nat).

lc_env_type(Env, pred(N), nat) :-
	lc_env_type(Env, N, nat).

lc_env_type(Env, iszero(N), bool) :-
	lc_env_type(Env, N, nat).

lc_env_type(Env, lambda(Var,Body), (TFrom->TTo)) :-
	lc_env_type([Var-TFrom|Env], Body, TTo).

lc_env_type(Env, apply(Lambda,Arg), TTo) :-
	lc_env_type(Env, Arg, TFrom),
	lc_env_type(Env, Lambda, (TFrom->TTo)).

lc_env_type(Env, var(Var), T) :-
	member(Var-T, Env).

lc_type(X, T) :-
	lc_env_type([], X, T).


% Helper relations

lc_not(true, false).
lc_not(false, true).

lc_if(Env, if(true,X,_), R) :-
	lc_env_eval(Env, X, R).
lc_if(Env, if(false,_,X), R) :-
	lc_env_eval(Env, X, R).

lc_pred(0, 0).
lc_pred(succ(N), N).

lc_iszero(0, true).
lc_iszero(succ(_), false).

lc_apply(closure(Env,Arg,Body), Val, R) :-
	lc_env_eval([Arg-Val|Env], Body, R).


% Interpreter
% TODO: How to make type-directed?

lc_env_eval(_, true, true).

lc_env_eval(_, false, false).

lc_env_eval(Env, not(X), R) :-
	lc_env_eval(Env, X, Y),
	lc_not(Y, R).

lc_env_eval(Env, if(Cond,Then,Else), R) :-
	lc_env_eval(Env, Cond, C),
	lc_if(Env, if(C,Then,Else), R).

lc_env_eval(_, 0, 0).

lc_env_eval(Env, succ(X), succ(R)) :-
	lc_env_eval(Env, X, R).

lc_env_eval(Env, pred(X), R) :-
	lc_env_eval(Env, X, N),
	lc_pred(N, R).

lc_env_eval(Env, iszero(X), R) :-
	lc_env_eval(Env, X, N),
	lc_iszero(N, R).

lc_env_eval(Env, lambda(Var,Body), closure(Env,Var,Body)).

lc_env_eval(Env, apply(X, Y), R) :-
	lc_env_eval(Env, X, X1),
	lc_env_eval(Env, Y, Y1),
	lc_apply(X1, Y1, R).

lc_env_eval(Env, var(Var), R) :-
	member(Var-R, Env).

lc_eval(X, R) :-
	lc_env_eval([], X, R).


% Iterative deepening setup

lc_serial(true) --> [true].
lc_serial(false) --> [false].
lc_serial(not(X)) --> [not], lc_serial(X).
lc_serial(if(X,Y,Z)) --> [if], lc_serial(X), lc_serial(Y), lc_serial(Z).
lc_serial(0) --> [0].
lc_serial(succ(X)) --> [succ], lc_serial(X).
lc_serial(pred(X)) --> [pred], lc_serial(X).
lc_serial(iszero(X)) --> [iszero], lc_serial(X).
lc_serial(lambda(X,Y)) --> [lambda], [X], lc_serial(Y).
lc_serial(apply(X,Y)) --> [apply], lc_serial(X), lc_serial(Y).
lc_serial(var(X)) --> [var, X].

lc_term_size(X, N) :-
	length(S, N),
	phrase(lc_serial(X), S).

lc_term(X) :-
	lc_term_size(X, _).


% Examples:
%
% Enumerate all terms:
% ?- lc_term(X).

% Enumerate all well-typed terms:
% ?- lc_term(X), lc_type(X, _).

% Synthesis of expressions that invert their argument:
% ?- lc_term(F), lc_eval(apply(F,true), false), lc_eval(apply(F,false), true).

% More synthesis tasks (fill in the blank):
% ?- lc_term(F), lc_type(F, (_->_)), F=lambda(a,if(var(a),_,_)), lc_eval(apply(F,true), false), lc_eval(apply(F,false), true).
% ?- lc_term(F), F=lambda(_,apply(_,_)), lc_type(F, (_->bool)), lc_eval(apply(F, lambda(a,not(var(a)))), true).

% Verification task - can this function ever return true?
% F=lambda(a,if(var(a),not(var(a)),var(a))), lc_eval(apply(F,X), true).
% ^ TODO: Does not terminate - how can it be made to?
%   Obvious solution is:
lc_bool(true).
lc_bool(false).
% ?- F=lambda(a,if(var(a),not(var(a)),var(a))), lc_bool(B), lc_eval(apply(F,B), true).

% Analogous relation for Nat:
lc_nat(0).
lc_nat(succ(X)) :-
	lc_nat(X).

% Conversion to CLP(FD) integer:
peano_num(0, 0).
peano_num(succ(P), N) :-
	N #> 0,
	N1 #= N - 1,
	peano_num(P, N1).

% Verification task - can this function ever return true?
% ?- F=lambda(x, iszero(succ(var(x)))), lc_eval(F, C), lc_apply(C, N, true).
%    ^ Interesting note: Trying to correct the evaluation behavior of succ or pred,
%      (i.e., by requiring their argument to be a ground Nat) yields non-termination here.

% Evaluation task - recursive `iseven` function:
% ?- F=lambda(x, if(iszero(var(x)), true, if(iszero(pred(var(x))), false, apply(F, pred(pred(var(x))))))), lc_eval(F, C), lc_apply(C, P, false), peano_num(P, N).

% Metatheory task - find terms that are not well-typed, but do evaluate:
% ?- lc_term(X), \+ lc_type(X, _), lc_eval(X, R).
