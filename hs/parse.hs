{-# LANGUAGE LambdaCase #-}

import Data.Char
import GHC.Base
import Data.Tuple
import System.Environment
import Data.List

newtype Parser a = Parser {
    runParser :: String -> Maybe (String, a)
}

type HTTPMessage = (String, [String], String)

show :: HTTPMessage -> String
show (begin, middle, end) = "start-line: \"" ++ begin ++ "\"\n"
    ++ "field-lines: " ++ (Prelude.show middle) ++ "\n"
    ++ "message-body: \"" ++ end ++ "\"\n"

-- Functor so awesome
instance Functor Parser where
    fmap f (Parser p) = Parser $ \input -> do
        (input', x) <- p input
        Just (input', f x)

instance Applicative Parser where
    pure x = Parser $ \input -> Just (input, x)
    (Parser a) <*> (Parser b) = Parser $ \input -> do
        (input', f) <- a input
        (input'', x) <- b input'
        Just (input'', f x)

instance Alternative Parser where
    empty = Parser $ \_ -> Nothing
    (Parser a) <|> (Parser b) = Parser $ \input -> a input <|> b input

-- [Parser helper functions]
parseAny :: Parser Char
parseAny = Parser f
    where
        f (x:xs) = Just (xs, x)
        f [] = Nothing

parseChar :: Char -> Parser Char
parseChar c = Parser $ \case
        x:xs | x == c -> Just (xs, x)
             | otherwise -> Nothing
        otherwise -> Nothing

parseAnyOf :: String -> Parser Char
parseAnyOf (x:xs) = parseChar x <|> parseAnyOf xs
parseAnyOf [] = empty

parseString :: String -> Parser String
parseString = sequenceA . map parseChar

parseSpan :: (Char -> Bool) -> Parser String
parseSpan f = Parser $ Just . swap . span f
parseBreak :: (Char -> Bool) -> Parser String
parseBreak f = Parser $ Just . swap . break f

parsePredicate :: (Char -> Bool) -> Parser Char
parsePredicate f = Parser $ \case
    (x:xs) -> if f x then Just (xs, x) else Nothing
    [] -> Nothing


placeholder :: Parser String
placeholder = parseString "foo"

-- [HTTP parsing business logic]
crlf :: Parser String
crlf = parseString "\r\n"

startLine :: Parser String
startLine = placeholder

fieldLine :: Parser String
fieldLine = placeholder

fieldLines :: Parser [String]
fieldLines = many (fieldLine <* crlf)

fieldVchar :: Parser Char
fieldVchar = vchar <|> obsText

vchar :: Parser Char
vchar = parsePredicate (\x -> if ord(x) >= 0x21 && 0x7E >= ord(x) then True else False)
obsText :: Parser Char
obsText = parsePredicate (\x -> if ord(x) >= 0x80 && 0xFF >= ord(x) then True else False)

-- optional whitespace
ows :: Parser String
ows = many $ parseAnyOf [ ' ', '\t' ]
-- required whitespace
rws :: Parser String
rws = some $ parseAnyOf [ ' ', '\t' ]

messageBody :: Parser String
messageBody = many parseAny


httpMessage :: String -> Maybe HTTPMessage
httpMessage input = do
    (input', cleanStartLine) <- runParser (startLine <* crlf) input
    (input'', cleanFieldLines) <- runParser (fieldLines <* crlf) input'
    (input''', cleanMessageBody) <- runParser messageBody input''
    Just (cleanStartLine, cleanFieldLines, cleanMessageBody)


main :: IO()
main = do
    args <- getArgs
    print args
    print . map (httpMessage) $ args
