;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Basic list utilities needed to implement macros
;;

(define list (lambda args args))
(define ident (lambda (x) x))

(define map
  ((lambda (cont) ; Note technique: let-over-lambda for default arg
     (lambda (f l cont) ; Note technique: continuation-passing style
       (if (atom l) (cont (if l (f l)))
	 (map f (cdr l) (lambda (x) (cont (cons (f (car l)) x)))))))
   ident))

(define assoc
  (lambda (s l)
    (if (atom l) ()
      (if (eq s (car (car l))) (car l)
	(assoc s (cdr l))))))

(define filter
  ((lambda (cont)
     (lambda (f l cont)
       (if (atom l) (cont (if l (if (f l) l)))
	 (if (f (car l))
	   (filter f (cdr l) (lambda (x) (cont (cons (car l) x))))
	   (filter f (cdr l) cont)))))
   ident))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Macro support
;;

; Which forms can be expanded? Not atoms, and not quotes
(define non-expandable
  (lambda (form)
    (if (atom form) t
      (if (eq (car form) 'quote) t))))

; Expand one level by (repeatedly) looking up and applying rules 
(define shallow-expand
  (lambda (form rules)
    (if (non-expandable form) form
      ((lambda (rule-fn args)
	 (if rule-fn (shallow-expand (rule-fn . args) rules)
	   form))
       (cdr (assoc (car form) rules))
       (cdr form)))))

; Expand all levels by expanding the current one first, then descending
(define deep-expand
  (lambda (form rules)
    (if (non-expandable form) form
      (map (lambda (x) (deep-expand x rules))
	   (shallow-expand form rules)))))

; Initialize expansion rules with defmacro for convenience
(define *expand-rules*
  (list
    (cons 'defmacro
	  (lambda (name/args body)
	    (list 'define '*expand-rules*
		  (list 'cons
			(list 'cons
			      (list 'quote (car name/args))
			      (list 'lambda (cdr name/args) body))
			'*expand-rules*))))))

; Enable macro expansion for all subsequent inputs
(define expand
  (lambda (form) (deep-expand form *expand-rules*)))

; Basic example: Logical not via comparison to nil
(defmacro (not x) (list 'eq () x))

; Translate complex list accessors into primitive ones
(defmacro (defchain name prim base)
  (list 'defmacro (list name 'x) ; Note technique: macro-defining macro
	(list 'list (list 'quote prim)
	      (list 'list (list 'quote base) 'x))))

(defchain caar car car) ; (Add to this list as needed) 
(defchain cadr car cdr)
(defchain cdar cdr car)
(defchain cddr cdr cdr)
(defchain cadar car cdar)
(defchain caddr car cddr)

; Translate cond into `if` chain
(defmacro (cond . qs-and-as)
  (if (atom qs-and-as) ()
    ; Optimize away t conditions
    (if (eq t (caar qs-and-as))
      ; (cond (t x) ...) => x
      (cadr (car qs-and-as))
      ; (cond (q x) ...) => (if q x (cond ...))
      (list 'if (caar qs-and-as) (cadar qs-and-as)
	    (cons 'cond (cdr qs-and-as)))))) ; Note technique: "recursive" macro

; Let-syntax
(defmacro (let binds body)
  (cons (list 'lambda (map (lambda (x) (car x)) binds) body)
	(map (lambda (x) (cadr x)) binds)))

; Enable `defun` syntax, with default arguments implemented by let-over-lambda
(defmacro (defun name/args body)
  (let ((name (car name/args))
	(args (map (lambda (x) (if (atom x) x (car x))) (cdr name/args)))
	(defaults (filter (lambda (x) (not (atom x)))   (cdr name/args))))
    (let ((fun (list 'lambda args body)))
      (list 'define name (if defaults (list 'let defaults fun) fun)))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Fun stuff
;;

; Applicative fixpoint combinator
(define Z
  (lambda (f)
    ((lambda (g) (f (lambda args ((g g) . args)))) ; or (lambda (g) (g g))
     (lambda (g) (f (lambda args ((g g) . args)))))))

; Variadic append in one definition via Z combinator
(defun (append . args)
  ((Z (lambda (append-cps)
	(lambda (cont l . ls)
	  (if (atom l) (if (atom ls) (cont l) (append-cps cont . ls))
	    (append-cps (lambda (x) (cont (cons (car l) x)))
			(cdr l) . ls)))))
   ident . args))

; List flattening via append
(defun (flatten l (cont ident))
  (cond ((atom l) (cont l))
	((atom (car l)) (flatten (cdr l) (lambda (x) (cont (cons (car l) x)))))
	(t (flatten (cdr l) (lambda (x) (cont (append (flatten (car l)) x)))))))

; Quasiquote syntax
(defmacro (` . args)
  ((Z (lambda (qq)
       (lambda (args)
	 (cond ((atom args) args)
	       ((not (atom (car args))) (list 'cons (qq (car args)) (qq (cdr args))))
	       ((eq ', (car args)) (list 'cons (cadr args) (qq (cddr args))))
	       ((eq ',@ (car args)) (list 'append (cadr args) (qq (cddr args))))
	       (t (list 'cons (list 'quote (car args)) (qq (cdr args))))))))
   args))
