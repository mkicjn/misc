; Executing this should not exhaust the stack or cell space if everything is working properly
;((lambda (f) (f f)) (lambda (f) (f f)))

; Y combinator test
(define Y
  (lambda (y)
    (let ((f (lambda (f) (y (lambda args ((f f) . args))))))
      (f f))))

(define last
  (Y (lambda (f)
       (lambda (ls) (cond ((atom? ls) ls)
			  ((null? (cdr ls)) (car ls))
			  ('else (f (cdr ls))))))))

(last '(a b c d e f g h i j k l m n o p q r s t u v w x y z))

((Y (lambda (f) (lambda () (f)))))
