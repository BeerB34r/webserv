module Main where

import Network.CGI (CGI, CGIResult, output, runCGI, handleErrors)

cgiMain :: CGI CGIResult
cgiMain = output "Hello, Haskell!"


main :: IO ()
main = runCGI (handleErrors cgiMain)
