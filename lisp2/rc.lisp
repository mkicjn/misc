;;;; Basic list utilities

(define list (lambda args args))

(define map
  ((lambda (cont) ; note let-over-lambda + continuation passing style
     (lambda (f l cont)
       (if (atom l) (cont l)
	 (map f (cdr l) (lambda (x) (cont (cons (f (car l)) x)))))))
   (lambda (x) x)))

(define assoc
  (lambda (s l)
    (if (atom l) ()
      (if (eq s (car (car l))) (car l)
	(assoc s (cdr l))))))


;;;; Macro expansion

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

; Enable `defun` with same syntax as `defmacro`
(defmacro (defun name/args body)
  (list 'define (car name/args) (list 'lambda (cdr name/args) body)))

; Translate longer list accessors into shorter ones (add as needed)
(defmacro (caar x) (list 'car (list 'car x)))
(defmacro (cadr x) (list 'car (list 'cdr x)))
(defmacro (cdar x) (list 'cdr (list 'car x)))
(defmacro (cddr x) (list 'cdr (list 'cdr x)))
(defmacro (cadar x) (list 'car (list 'cdar x)))
(defmacro (caddr x) (list 'car (list 'cddr x)))
(defmacro (cddar x) (list 'cdr (list 'cdar x)))
(defmacro (cdddr x) (list 'cdr (list 'cddr x)))
(defmacro (caddar x) (list 'car (list 'cddar x)))
(defmacro (cadddr x) (list 'car (list 'cdddr x)))

; Translate cond into `if` chain
(defmacro (cond . qs-and-as)
  (if (atom qs-and-as) ()
    (list 'if (caar qs-and-as) (cadar qs-and-as)
	  (cons 'cond (cdr qs-and-as)))))
