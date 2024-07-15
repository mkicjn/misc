; Testing currying, closures, TCO
(define curry (lambda (f x) (lambda args (f x . args))))
(define assp (lambda (f l) (cond ((null? l) '()) ((f (car (car l))) (cdr (car l))) ('t (assp f (cdr l))))))
(define assoc (lambda (e l) (assp (curry eq? e) l)))
(assoc 'z '((a . 1) (b . 2) (c . 3) (d . 4) (e . 5) (f . 6) (g . 7) (h . 8) (i . 9) (j . 10) (l . 11) (m . 12) (n . 13)
            (o . 14) (p . 15) (q . 16) (r . 17) (s . 18) (t . 19) (u . 20) (v . 21) (w . 22) (x . 23) (y . 24) (z . 25)))

; Testing sequential let bindings
(let ((a '1) (b (cons a '2)) (a '0)) (cons a b))

; Demo: Issue with GC - deep copying prevents consistent equality checking for cells
(let ((a '(1))          (b a) (c '(2))) (eq? a b))
(let ((a (cons '1 '())) (b a) (c '(2))) (eq? a b))
; Same issue without let
((lambda (x) ((lambda (a b) (eq? a b)) x x)) (cons '1 '()))
((lambda (p) (eq? (car p) (cdr p))) ((lambda (x) (cons x x)) (cons '1 '())))
