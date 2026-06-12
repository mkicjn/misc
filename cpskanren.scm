; (Unfinished) μKanren-like using CPS instead of streams

; Variables and unification

(define (var x) (vector x))
(define (var? x) (vector? x))

(define (walk v s)
  (if (not (var? v)) v
    (let ((m (assoc v s)))
      (if m (walk (cdr m) s) v))))

(define (occurs? a b)
  (cond ((eq? a b) #t)
	((list? b) (or (occurs? a (car b))
		       (occurs? a (cdr b))))
	(else #f)))

(define (ext-s a b s)
  (if (occurs? a b) #f
    (cons (cons a b) s)))

(define (unify a b s)
  (let ((a (walk a s))
	(b (walk b s)))
    (cond ((eq? a b) s)
	  ((var? a) (ext-s a b s))
	  ((var? b) (ext-s b a s))
	  ((atom? a) #f)
	  ((atom? b) #f)
	  (else
	    (let* ((s (unify (car a) (car b) s)))
	      (if s (unify (cdr a) (cdr b) s)))))))


; The hard part - using continuations instead of streams

(define (ident x) x)

(define (== a b)
  (lambda (k)
    (lambda (s)
      (let ((s (unify a b s)))
	(if s (k s))))))

(define (conj g1 g2)
  (lambda (k)
    (lambda (s)
      ((g2 (g1 k)) s))))


; TODO: Try implementing some kind of `take` analogue
; TODO: Implement disj
; TODO: Does this architecture make a fair variadic disj easier?
