collatzStep :: Int -> Int
collatzStep n =
  case n `mod` 2 of
    0 -> n `div` 2;
    1 -> 3 * n + 1

collatzLen :: Int -> Int
collatzLen 1 = 0
collatzLen n =
  (1+) . collatzLen $ collatzStep n

collatzMax :: Int -> Int
collatzMax lim =
  foldl max 0 $ map collatzLen [1..lim]

main = print $ collatzMax 1000000
