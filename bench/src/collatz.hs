collatzLen :: Int -> Int
collatzLen 1 = 0
collatzLen n =
  (1+) $ collatzLen $ step n
  where step n
         | even n     = n `div` 2
         | otherwise  = 3 * n + 1

main = print $ maximum $ map collatzLen [1..1000000]
