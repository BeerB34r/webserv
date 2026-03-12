Simple webserver in cpp, written by [mde-beer], [mifelida], and [amel-fou] during
studies at [Codam]

# Resources

[HTTP/1.1 specification](https://www.rfc-editor.org/rfc/rfc9112), related documents hyperlinked within

# Notes
## CGI related
binary files are binary files and can be ran regardless of source language

haskell files can be intepreted and ran directly as follows:
```sh
ghc --run <filename> -- [parameters...]
```
haskell has a CGI library under Network.CGI

haskell shebang:
```hs
#!/usr/bin/env -S ghc --run
```

## TODO
### cookies
- all of it
### multiple cgi types
- should be trivial, but all of it
### config
- interface spec
- default error pages
- max allowed request body
- accepted methods
- file extension cgi
- default file when directory
- enable/disable directory listing
- upload file permission + storage location

