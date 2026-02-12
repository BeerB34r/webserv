{-# LANGUAGE LambdaCase #-}

import Data.Char
import Data.Tuple
import Data.List
import Control.Applicative

import System.Environment

newtype Parser a = Parser {
    runParser :: String -> Maybe (String, a)
}

type HTTPMessage = (String, [String], String)

show :: HTTPMessage -> String
show (begin, middle, end) = "start-line: \"" ++ begin ++ "\"\n"
    ++ "field-lines: " ++ Prelude.show middle ++ "\n"
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
    empty = Parser $ const Nothing
    (Parser a) <|> (Parser b) = Parser $ \input -> a input <|> b input

-- [Parser helper functions]
parseOptional :: a -> Parser a -> Parser a
parseOptional x p1 = p1 <|> pure x

parseAny :: Parser Char
parseAny = Parser f
    where
        f (x:xs) = Just (xs, x)
        f [] = Nothing

parseChar :: Char -> Parser Char
parseChar c = Parser $ \case
        x:xs | x == c -> Just (xs, x)
             | otherwise -> Nothing
        _ -> Nothing

parseAnyOf :: String -> Parser Char
parseAnyOf = foldr ((<|>) . parseChar) empty

parseString :: String -> Parser String
parseString = traverse parseChar

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

startLine :: Parser String
startLine = requestLine <|> statusLine

requestLine :: Parser String
requestLine = (\a b c d e -> a ++ b ++ c ++ d ++ e) <$>
    method <*> (singleton <$> parseChar ' ') <*>
    requestTarget <*> (singleton <$> parseChar ' ') <*>
    httpVersion

requestTarget :: Parser String
requestTarget = originForm <|> absoluteForm <|> authorityForm <|> asteriskForm
originForm :: Parser String
originForm = (++) <$> absolutePath <*> parseOptional "" ((:) <$> parseChar '?' <*> query)
absoluteForm :: Parser String
absoluteForm = absoluteURI
authorityForm :: Parser String
authorityForm = undefined
asteriskForm :: Parser String
asteriskForm = undefined

statusLine :: Parser String
statusLine = undefined

absolutePath :: Parser String
absolutePath = concat <$> some ((:) <$> parseChar '/' <*> segment)

absoluteURI :: Parser String
absoluteURI =  (++) <$> scheme <*> ((:) <$> parseChar ':' <*> ((++) <$> hierPart <*> parseOptional "" ((:) <$> parseChar '?' <*> query)))

scheme :: Parser String
scheme = (:) <$> parsePredicate isAlpha <*> many (parsePredicate isAlpha <|> parsePredicate isDigit <|> parseAnyOf ['+', '-', '.'])

hierPart :: Parser String
hierPart = ((++) <$> parseString "//" <*> ((++) <$> authority <*> pathAbempty))
    <|> pathAbsolute
    <|> pathRootless
    <|> pathEmpty

authority :: Parser String
authority = (++) <$> parseOptional "" (flip (:) <$> userinfo <*> parseChar '@') <*> 
    ((++) <$> host <*>
    parseOptional "" ((:) <$> parseChar ':' <*> port))
userinfo :: Parser String
userinfo = concat <$> many (singleton <$> unreserved <|> pctEncoded <|> singleton <$> subDelims <|> singleton <$> parseChar ':')
host :: Parser String
host = ipLiteral <|> ipv4address <|> regName
ipLiteral :: Parser String
ipLiteral = (:) <$> parseChar '[' <*> (flip (:) <$> (ipv6address <|> ipvfuture) <*> parseChar ']')
ipv6address :: Parser String
ipv6address = undefined
ipvfuture :: Parser String
ipvfuture = (:) <$> parseChar 'v' <*> ((++) <$> some (parsePredicate isHexDigit) <*> ((:) <$> parseChar '.' <*> some (unreserved <|> subDelims <|> parseChar ':')))
ipv4address :: Parser String
ipv4address = (\a b c d e f g -> a ++ b ++ c ++ d ++ e ++ g) <$> decOctet <*> (singleton <$> parseChar '.') <*> decOctet <*> (singleton <$> parseChar '.') <*> decOctet <*> (singleton <$> parseChar '.') <*> decOctet
regName :: Parser String
regName = concat <$> many (singleton <$> unreserved <|> pctEncoded <|> singleton <$> subDelims)
port :: Parser String
port = many (parsePredicate isDigit)
decOctet :: Parser String
decOctet = (\a b c -> [a,b,c]) <$> parseChar '1' <*> parsePredicate isDigit <*> parsePredicate isDigit
    <|> (\a b c -> [a,b,c]) <$> parseChar '2' <*> parseAnyOf "01234" <*> parsePredicate isDigit
    <|> flip (:) <$> parseString "25" <*> parseAnyOf "012345"
    <|> (\a b -> [a,b]) <$> parseAnyOf "123456789" <*> parsePredicate isDigit
    <|> singleton <$> parsePredicate isDigit

pathAbempty :: Parser String
pathAbempty = undefined
pathAbsolute :: Parser String
pathAbsolute = undefined
pathRootless :: Parser String
pathRootless = undefined
pathEmpty :: Parser String
pathEmpty = undefined

segment :: Parser String
segment = concat <$> many pchar

query :: Parser String
query = concat <$> many (pchar <|> singleton <$> parseChar '/' <|> singleton <$> parseChar '?')

pchar :: Parser String
pchar = (singleton <$> unreserved) <|> pctEncoded <|> (singleton <$> subDelims) <|> (singleton <$> parseChar ':') <|> (singleton <$> parseChar '@')

unreserved :: Parser Char
unreserved = parsePredicate isAlpha <|> parsePredicate isDigit <|> parseAnyOf ['-', '.', '_', '~']

pctEncoded :: Parser String
pctEncoded = (\a b c -> [a, b, c]) <$> parseChar '%' <*> parsePredicate isHexDigit <*> parsePredicate isHexDigit

subDelims :: Parser Char
subDelims = parseAnyOf ['!', '$', '&', '\\', '(', ')', '*', '+', ',', ':', '=' ]

httpVersion :: Parser String
httpVersion = (\a b c d e -> a ++ b ++ c ++ d ++ e) <$> parseString "HTTP" <*> (singleton <$> parseChar '/') <*> (singleton <$> parsePredicate isDigit) <*> (singleton <$> parseChar '.') <*> (singleton <$> parsePredicate isDigit)

fieldLines = many (fieldLine <* crlf)

fieldLine :: Parser String
fieldLine = (\x y z -> x ++ y ++ z) <$> fieldName <*> ((: []) <$> parseChar ':') <*> (ows *> fieldValue <* ows)

fieldName :: Parser String
fieldName = token

fieldValue :: Parser String
fieldValue = concat <$> many fieldContent

fieldContent :: Parser String
fieldContent = ((++) . singleton <$> fieldVchar) <*> optionalPart
    where optionalPart = parseOptional "" content
            where
                content = Parser $ \input -> do
                    (input', x) <- runParser (some (parseAnyOf [' ', '\t'] <|> fieldVchar)) input
                    (dropped, got) <- runParser (many $ parseAnyOf [' ', '\t']) . reverse $ x
                    Just (reverse got ++ input', reverse dropped)

fieldVchar :: Parser Char
fieldVchar = vchar <|> obsText


tchar :: Parser Char
tchar = parseAnyOf ['!', '#', '$', '%', '&', '\\', '*', '+', '-', '.', '^', '_', '`', '|', '~' ] <|> parsePredicate isDigit <|> parsePredicate isAlpha
vchar :: Parser Char
vchar = parsePredicate (\x -> ord x >= 0x21 && 0x7E >= ord x)
obsText :: Parser Char
obsText = parsePredicate (\x -> ord x >= 0x80 && 0xFF >= ord x)

-- optional whitespace
ows :: Parser String
ows = many $ parseAnyOf [ ' ', '\t' ]
-- required whitespace
rws :: Parser String
rws = some $ parseAnyOf [ ' ', '\t' ]
-- carriage return line feed
crlf :: Parser String
crlf = parseString "\r\n"

method :: Parser String
method = token
token :: Parser String
token = some tchar

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
    print . map httpMessage $ args
