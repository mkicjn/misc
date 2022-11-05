(defun collatz (n)
  (declare (optimize (speed 3) (safety 0)) (fixnum n))
  (loop for i fixnum from 0
	until (<= n 1)
	do (setf n (if (evenp n) (/ n 2) (1+ (* 3 n))))
	finally (return i)))

(defun maxlen (n)
  (declare (optimize (speed 3) (safety 0)) (fixnum n))
  (loop for i from 0 to n
	maximize (collatz i)))

(princ (maxlen 1000000))
