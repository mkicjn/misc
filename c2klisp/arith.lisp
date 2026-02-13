; WIP implementation of languages from Ch. 3 of TAPL (Pierce)
; Run with, e.g., `./c2klisp.c rc.lisp arith.lisp -`

;; Reflexive transitive closure over single-step evaluation

; TODO: Technically only a transitive closure, since each single-step is already reflexive
; See if the single-step eval can return 'stuck or something when no derivations are possible

(defun (make->* ->)
  (Z (lambda (->*)
       (lambda (t0)
	 (let ((t0` (-> t0)))
	   (if (equal t0` t0) t0` (->* t0`)))))))


;; Bool language (B)

(defun (-B-> t0)
  (match t0
	 ((if true then , t2 else _) t2)
	 ((if false then _ else , t3) t3)
	 ((if , t1 then , t2 else , t3) (` if , (-B-> t1) then , t2 else , t3))
	 (_ t0)))

(define -B->* (make->* -B->))

; (tests)
(eq (-B->* '(if true then (if (if false then false else true) then false else true) else false))
    'false)


;; Nat language (NB - extends B)

(defun (is-nv t0)
  (match t0
	 (zero t)
	 ((succ , t1) (is-nv t1))))

(defun (-NB-> t0)
  (match t0
	 ((if true then , t2 else _) t2)
	 ((if false then _ else , t3) t3)
	 ((if , t1 then , t2 else , t3) (` if , (-NB-> t1) then , t2 else , t3))
	 ((succ , t1) t0 when (is-nv t1))
	 ((succ , t1) (` succ , (-NB-> t1)))
	 ((pred zero) 'zero)
	 ((pred (succ , nv)) nv when (is-nv nv))
	 ((pred , t1) (` pred , (-NB-> t1)))
	 ((iszero zero) 'true)
	 ((iszero (succ nv)) 'false when (is-nv nv))
	 ((iszero , t1) (` iszero , (-NB-> t1)))
	 (_ t0)))

(define -NB->* (make->* -NB->))

; (tests)
(is-nv 'zero)
(is-nv '(succ (succ zero)))

(equal (-NB->* '(if (iszero (pred (succ zero))) then (succ zero) else zero))
       '(succ zero))
