-- collatz.hs with lazy lists
import Data.List (elemIndex)

collatz :: Int -> [Int]
collatz = iterate step
  where step n
         | even n    = n `div` 2
         | otherwise = 3 * n + 1

main = print $ maximum . map (elemIndex 1) $ map collatz [1..1000000]
