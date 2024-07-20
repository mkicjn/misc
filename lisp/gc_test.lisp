; Demo: Issue with GC - deep copying prevents consistent equality checking for cells
(let* ((a '(1))          (b a) (c '(2))) (eq? a b))
(let* ((a (cons '1 '())) (b a) (c '(2))) (eq? a b))
; Same issue without let
((lambda (x) ((lambda (a b) (eq? a b)) x x)) (cons '1 '()))
((lambda (p) (eq? (car p) (cdr p))) ((lambda (x) (cons x x)) (cons '1 '())))
