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
#### done
#### todo
- all of it
### multiple cgi types
#### done
- currently supports:
    - binary
    - haskell
    - python
    - shell
    - .cgi (anything that can run directly)
#### wip
#### todo
- environment
- nonblocking

### config
#### done
- interface spec => done
- default error pages => resolved on per-server basis, configured in special block
- default file when directory => index key
- enable/disable directory listing => autoindex key
- add doctype to headers
- accepted methods
- cgi => gets content (blocking)
- upload file permission + storage location
- max allowed request body => requestHandling now inside readEvent
