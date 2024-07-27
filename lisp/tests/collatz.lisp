; This Collatz sequence benchmark runs about 16% faster here than in the CHICKEN interpreter
; It's still about 87x slower than paraforth, though ;)
(define collatz
  (lambda (n acc) (cond ((= n 1) acc)
			((= (mod n 2) 0) (collatz (/ n 2) (+ acc 1)))
			(t (collatz (+ 1 (* 3 n)) (+ acc 1))))))

(define collatz-max
  (lambda (n m) (cond ((= n 1) m)
		      (t (collatz-max (- n 1) (max (collatz n 0) m))))))

(collatz-max 1000000 0)
