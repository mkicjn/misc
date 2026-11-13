import Control.Monad

type Graph a = [(a, [a])]

keys :: Graph a -> [a]
keys = map fst

toposort :: Eq a => Graph a -> Maybe [a]
toposort [] = Just []
toposort g = case candidates g of
             []    -> Nothing
             (x:_) -> fmap (x:) $ toposort $ g `without` x
  where candidates :: Eq a => Graph a -> [a]
        candidates g = [x | (x, ys) <- g, ys == []]
        without :: Eq a => Graph a -> a -> Graph a
        [] `without` _ = []
        ((x,ys):g) `without` x'
         | x == x'    = g `without` x'
         | otherwise  = (x, [y | y <- ys, y /= x']) : (g `without` x')
 

addVertex :: Eq a => a -> Graph a -> Graph a
addVertex x [] = [(x, [])]
addVertex x g@(e@(y, ys):g')
 | y == x    = g
 | otherwise = e : addVertex x g'

addEdge :: Eq a => (a, a) -> Graph a -> Graph a
addEdge e@(x0, x1) = addEdge' e . addVertex x0 . addVertex x1
  where addEdge' e@(x0, x1) (es@(x, xs):g)
         | x == x0   = (x, x1:xs):g
         | otherwise = es : addEdge' e g

graphFromEdges :: Eq a => [(a, a)] -> Graph a
graphFromEdges = foldl (flip addEdge) []

--x = graphFromEdges [("x", "y"), ("y", "z"), ("y", "w"), ("x", "w")]


--

type Name = String
type Time = Int
type Dependency = (String, (Maybe Int))
type Task = (Name, (Time, [Dependency]))

graphFromTasks :: [Task] -> Graph Name
graphFromTasks = graphFromEdges . concat . map toEdges
  where toEdges :: Task -> [(Name, Name)]
        toEdges (n, (_, ds)) = [(n, n') | d <- ds, let n' = fst d]

x = [("A", (6, []))
    ,("B", (3, [("A", Just 3)]))
    ,("C", (1, [("B", Nothing)]))]

-- main = print $ toposort $ graphFromTasks x

type ScheduledTask = (Name, (Time, Time))
schedule :: [Task] -> Maybe [ScheduledTask]
schedule ts = fmap reverse $ (toposort $ graphFromTasks ts) >>= foldM update []
  where update :: [ScheduledTask] -> Name -> Maybe [ScheduledTask]
        update ss n =
          case lookup n ts of
          Just (dt, ds) -> let start = fmap (foldl max 0) $ sequence $ map (getStart ss) ds
                           in case start of
                             Nothing -> Nothing
                             Just t0 -> Just ((n, (t0, t0 + dt)) : ss)
          Nothing -> Nothing
        getStart :: [ScheduledTask] -> Dependency -> Maybe Time
        getStart ss (n, Just t)  = fmap ((t+) . fst) $ lookup n ss
        getStart ss (n, Nothing) = fmap         snd  $ lookup n ss


main = print $ schedule x
