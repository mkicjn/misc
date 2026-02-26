let collatz n =
  let rec f n i =
    match n with
    | n when n <= 1 -> i
    | n when n mod 2 = 0 -> f  (n / 2)  (i + 1)
    | n when n mod 2 = 1 -> f (3*n + 1) (i + 1)
    | _ -> 0
  in f n 0
;;

let maxlen lim =
  let rec f lim best =
    if lim <= 1 then best
    else let b = collatz lim in
      f (lim - 1) (if b > best then b else best)
  in f lim 0
;;

Printf.printf "%d" (maxlen 1000000)
