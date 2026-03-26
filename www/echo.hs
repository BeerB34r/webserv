import System.IO

main :: IO()
main = do
    putStr "\r\n"
    putStrLn =<< hGetContents' stdin
