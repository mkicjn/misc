% Relational interpreter experiment
% (For simply-typed λ-calculus with Bool and Nat)
% (Also, with type inference)

:- use_module(library(lists)).
:- use_module(library(clpfd)).

% Typing rules

lb_env_type(_, true, bool).
lb_env_type(_, false, bool).

lb_env_type(Env, not(X), bool) :-
	lb_env_type(Env, X, bool).

lb_env_type(Env, if(A,B,C), T) :-
	lb_env_type(Env, A, bool),
	lb_env_type(Env, B, T),
	lb_env_type(Env, C, T).

lb_env_type(_, 0, nat).
lb_env_type(Env, succ(N), nat) :-
	lb_env_type(Env, N, nat).

lb_env_type(Env, pred(N), nat) :-
	lb_env_type(Env, N, nat).

lb_env_type(Env, iszero(N), bool) :-
	lb_env_type(Env, N, nat).

lb_env_type(Env, lambda(Var,Body), (TFrom->TTo)) :-
	lb_env_type([Var-TFrom|Env], Body, TTo).

lb_env_type(Env, apply(Lambda,Arg), TTo) :-
	lb_env_type(Env, Arg, TFrom),
	lb_env_type(Env, Lambda, (TFrom->TTo)).

lb_env_type(Env, var(Var), T) :-
	member(Var-T, Env).

lb_type(X, T) :-
	lb_env_type([], X, T).


% Helper relations

lb_not(true, false).
lb_not(false, true).

lb_if(Env, if(true,X,_), R) :-
	lb_env_eval(Env, X, R).
lb_if(Env, if(false,_,X), R) :-
	lb_env_eval(Env, X, R).

lb_pred(0, 0).
lb_pred(succ(N), N).

lb_iszero(0, true).
lb_iszero(succ(_), false).

lb_apply(closure(Env,Arg,Body), Val, R) :-
	lb_env_eval([Arg-Val|Env], Body, R).


% Interpreter
% TODO: How to make type-directed?

lb_env_eval(_, true, true).

lb_env_eval(_, false, false).

lb_env_eval(Env, not(X), R) :-
	lb_env_eval(Env, X, Y),
	lb_not(Y, R).

lb_env_eval(Env, if(Cond,Then,Else), R) :-
	lb_env_eval(Env, Cond, C),
	lb_if(Env, if(C,Then,Else), R).

lb_env_eval(_, 0, 0).

lb_env_eval(Env, succ(X), succ(R)) :-
	lb_env_eval(Env, X, R).

lb_env_eval(Env, pred(X), R) :-
	lb_env_eval(Env, X, N),
	lb_pred(N, R).

lb_env_eval(Env, iszero(X), R) :-
	lb_env_eval(Env, X, N),
	lb_iszero(N, R).

lb_env_eval(Env, lambda(Var,Body), closure(Env,Var,Body)).

lb_env_eval(Env, apply(X, Y), R) :-
	lb_env_eval(Env, X, X1),
	lb_env_eval(Env, Y, Y1),
	lb_apply(X1, Y1, R).

lb_env_eval(Env, var(Var), R) :-
	member(Var-R, Env).

lb_eval(X, R) :-
	lb_env_eval([], X, R).


% Iterative deepening setup

lb_serial(true) --> [true].
lb_serial(false) --> [false].
lb_serial(not(X)) --> [not], lb_serial(X).
lb_serial(if(X,Y,Z)) --> [if], lb_serial(X), lb_serial(Y), lb_serial(Z).
lb_serial(0) --> [0].
lb_serial(succ(X)) --> [succ], lb_serial(X).
lb_serial(pred(X)) --> [pred], lb_serial(X).
lb_serial(iszero(X)) --> [iszero], lb_serial(X).
lb_serial(lambda(X,Y)) --> [lambda], [X], lb_serial(Y).
lb_serial(apply(X,Y)) --> [apply], lb_serial(X), lb_serial(Y).
lb_serial(var(X)) --> [var, X].

lb_term_size(X, N) :-
	length(S, N),
	phrase(lb_serial(X), S).

lb_term(X) :-
	lb_term_size(X, _).


% Examples:
%
% Enumerate all terms:
% ?- lb_term(X).

% Enumerate all well-typed terms:
% ?- lb_term(X), lb_type(X, _).

% Synthesis of expressions that invert their argument:
% ?- lb_term(F), lb_eval(apply(F,true), false), lb_eval(apply(F,false), true).

% More synthesis tasks (fill in the blank):
% ?- lb_term(F), lb_type(F, (_->_)), F=lambda(a,if(var(a),_,_)), lb_eval(apply(F,true), false), lb_eval(apply(F,false), true).
% ?- lb_term(F), F=lambda(_,apply(_,_)), lb_type(F, (_->bool)), lb_eval(apply(F, lambda(a,not(var(a)))), true).

% Verification task - can this function ever return true?
% F=lambda(a,if(var(a),not(var(a)),var(a))), lb_eval(apply(F,X), true).
% ^ TODO: Does not terminate - how can it be made to?
%   Obvious solution is:
lb_bool(true).
lb_bool(false).
% ?- F=lambda(a,if(var(a),not(var(a)),var(a))), lb_bool(B), lb_eval(apply(F,B), true).

% Analogous relation for Nat:
lb_nat(0).
lb_nat(succ(X)) :-
	lb_nat(X).

% Conversion to CLP(FD) integer:
peano_num(0, 0).
peano_num(succ(P), N) :-
	N #> 0,
	N1 #= N - 1,
	peano_num(P, N1).

% Verification task - can this function ever return true?
% ?- F=lambda(x, iszero(succ(var(x)))), lb_eval(F, C), lb_apply(C, N, true).

% Evaluation task - recursive `iseven` function:
% ?- F=lambda(x, if(iszero(var(x)), true, if(iszero(pred(var(x))), false, apply(F, pred(pred(var(x))))))), lb_eval(F, C), lb_apply(C, P, false), peano_num(P, N).
