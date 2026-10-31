import Data.Bits

type Frac = (Integer, Integer)

fractran :: Integer -> [Frac] -> [Integer]
fractran _ [] = []
fractran i fs = go fs
  where go [] = [i]
        go ((n, d):fs') =
          let (q, r) = (i * n) `divMod` d
          in if r == 0 then i : fractran q fs else go fs'

primes = [(17, 91),
          (78, 85),
          (19, 51),
          (23, 38),
          (29, 33),
          (77, 29),
          (95, 23),
          (77, 19),
          ( 1, 17),
          (11, 13),
          (13, 11),
          (15,  2),
          ( 1,  7),
          (55,  1)]

main = sequence $ map (print . lg) $ filter isPow2 $ fractran 2 primes
  where isPow2 = (==1) . popCount
        lg 1 = 0
        lg n = 1 + (lg (n `shiftR` 1))
