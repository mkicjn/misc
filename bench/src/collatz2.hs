
import Data.List (elemIndex)

collatz = iterate step
  where step n = case n `mod` 2 of
                 0 -> n `div` 2;
                 1 -> 3 * n + 1

collatzLen = elemIndex 1 . collatz

main = print $ maximum $ map collatzLen [1..1000000]
