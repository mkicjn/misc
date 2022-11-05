; csc -O5 collatz.scm
(define (collatz n)
  (let collatz-rec ((n n) (i 0))
    (cond ((<= n 1)           i)
	  ((= 0 (modulo n 2)) (collatz-rec (/ n 2)       (+ 1 i)))
	  (#t                 (collatz-rec (+ 1 (* 3 n)) (+ 1 i))))))

(define (maxlen lim)
  (let maxlen-rec ((i 0) (m 0))
    (let ((c (collatz i)))
      (cond ((= i lim) m)
	    ((< m c)   (maxlen-rec (+ 1 i) c))
	    (#t        (maxlen-rec (+ 1 i) m))))))

(display (maxlen 1000000))
(exit)
