import Control.Monad
import Data.List

type Graph a = [(a, [a])]

keys :: Graph a -> [a]
keys = map fst

toposort :: Eq a => Graph a -> Maybe [a]
toposort [] = Just []
toposort g = case candidates g of
             []    -> Nothing
             (x:_) -> fmap (x:) $ toposort $ g `without` x
  where
    candidates :: Eq a => Graph a -> [a]
    candidates g = [x | (x, ys) <- g, null ys]

    without :: Eq a => Graph a -> a -> Graph a
    [] `without` _ = []
    ((x,ys):g) `without` x'
     | x == x'    = g `without` x'
     | otherwise  = (x, filter (/=x') ys) : (g `without` x')


addVertex :: Eq a => Graph a -> a -> Graph a
addVertex [] x = [(x, [])]
addVertex g@(e@(y, ys):g') x
 | y == x    = g
 | otherwise = e : addVertex g' x

addEdge :: Eq a => Graph a -> (a, a) -> Graph a
addEdge g e@(x0, x1) = addE (addVertex (addVertex g x0) x1)
  where addE (es@(x, xs):g)
         | x == x0   = (x, x1:xs):g
         | otherwise = es : addE g


--

type Name = String
type Time = Int
data Delay = Complete | After Time
type Dependency = (String, Delay)
type Task = (Name, Time, [Dependency])
type ScheduledTask = (Name, (Time, Time))

graphFromTasks :: [Task] -> Graph Name
graphFromTasks ts =
  let withVs = foldl addVertex [] $ map toVertex ts
  in foldl addEdge withVs $ concat $ map toEdges ts
  where toVertex :: Task -> Name
        toVertex (n, _, _) = n
        toEdges :: Task -> [(Name, Name)]
        toEdges (n, _, ds) = map ((n,) . fst) ds

schedule :: [Task] -> Maybe [ScheduledTask]
schedule ts = fmap reverse $ (toposort $ graphFromTasks ts) >>= foldM update []
  where update :: [ScheduledTask] -> Name -> Maybe [ScheduledTask]
        update ss n =
          case find (\(x,_,_) -> x == n) ts of
          Just (_, dt, ds) -> let start = fmap (foldl max 0) $ sequence $ map (getStart ss) ds
                              in case start of
                                Nothing -> Nothing
                                Just t0 -> Just ((n, (t0, t0 + dt)) : ss)
          Nothing -> Nothing
        getStart :: [ScheduledTask] -> Dependency -> Maybe Time
        getStart ss (n, After t)  = fmap ((t+) . fst) $ lookup n ss
        getStart ss (n, Complete) = fmap         snd  $ lookup n ss


--

main :: IO ()
main = putStr $ maybe "Invalid input\n" showSchedule $ schedule tasks
  where
    showSchedule :: [ScheduledTask] -> String
    showSchedule = unlines . map bar . maybe id rescale cols

    rescale :: Int -> [ScheduledTask] -> [ScheduledTask]
    rescale i ts =
      let frac = (fromIntegral i) / (fromIntegral (endTime ts))
      in map (\(n, (start, end)) ->
                (n, (floor   (frac * (fromIntegral start)),
                     ceiling (frac * (fromIntegral end))))) ts

    endTime :: [ScheduledTask] -> Time
    endTime = foldl max 0 . map (\(_, (_, n)) -> n)

    bar :: ScheduledTask -> String
    bar (n, (start, end)) =
      let len = end - start
      in concat [n, sep, concat $ replicate start space,
                         concat $ replicate len pip]


--

sep :: String
sep = "\t|"

space :: String
space = " "

pip :: String
pip = "."

cols :: Maybe Int
cols = Just 40

tasks :: [Task]
tasks = [("A", 6, [])
        ,("B", 3, [("A", After 3)])
        ,("C", 1, [("B", Complete)])
        ,("D", 4, [("B", After 1)])]
