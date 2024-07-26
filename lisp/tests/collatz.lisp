; This Collatz sequence benchmark takes ~1.6x as long to run than in the CHICKEN interpreter
(define collatz
  (lambda (n acc) (cond ((= n 1) acc)
			((= (mod n 2) 0) (collatz (/ n 2) (+ acc 1)))
			(t (collatz (+ 1 (* 3 n)) (+ acc 1))))))

(define collatz-max
  (lambda (n m) (cond ((= n 1) m)
		      (t (collatz-max (- n 1) (max (collatz n 0) m))))))

(collatz-max 100000 0) ; Shortened from the usual 1000000
