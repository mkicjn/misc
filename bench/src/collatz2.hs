collatz :: Int -> [Int]
collatz = iterate step
  where step n
         | even n    = n `div` 2
         | otherwise = 3 * n + 1

main = print $ maximum $ map (length . takeWhile (/=1) . collatz) [1..1000000]
