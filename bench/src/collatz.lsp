; newLISP test
(define (collatz n)
  (set 'i 0)
  (do-until (<= n 1)
	    (set 'n (if (= (mod n 2) 0) (/ n 2) (+ 1 (* 3 n))))
	    (set 'i (+ 1 i)))
  i)

(define (maxlen lim)
  (set 'm 0)
  (dotimes (i lim)
    (set 'm (max m (collatz i))))
  m)

(print (maxlen 1000000))
(exit)
