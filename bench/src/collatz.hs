collatz :: Int -> Int
collatz n = collatz' 0 n
  where collatz' acc n
         | n == 1     = acc
         | even n     = collatz' (acc+1) (n `div` 2)
         | otherwise  = collatz' (acc+1) (3 * n + 1)

main = print $ maximum $ map collatz [1..1000000]
