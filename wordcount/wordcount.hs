import Data.Function
import Data.Map (Map, alter, empty, assocs)
import Data.List (sortBy)
import Data.Tuple (swap)
import System.Environment
import System.IO
import Data.Char
import Data.Ord

main = do
  args <- getArgs
  handle <- if null args
            then return System.IO.stdin
            else openFile (args !! 0) ReadMode
  contents <- hGetContents handle
  putStrLn (map normalize contents
           & lines & map words & concat
           & foldl count (empty :: Map String Int)
           & assocs & sortBy (comparing (\(x, y) -> (Down y, x)))
           & map (\(w, n) -> show n ++ "\t" ++ w)
           & unlines)
  where 
    normalize :: Char -> Char
    normalize c
     | isLetter c = toLower c
     | otherwise  = ' '

    count :: Map String Int -> String -> Map String Int
    count = flip $ alter (\n -> case n of
                                Nothing -> Just 1
                                Just x -> Just $ x+1)
