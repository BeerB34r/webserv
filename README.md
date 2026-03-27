*This project has been created as part of the 42 curriculum by mde-beer*

Simple webserver in C++, written by [mde-beer](https://profile.intra.42.fr/users/mde-beer) during studies at [Codam/42 Amsterdam](codam.nl)

# Description
Webserv is the first real introduction to socket programming within the core
curriculum. The idea is to create a small nginx clone that is capable of serving
static websites as well as handling CGI scripts.
Amongst the formal restrictions are that the entire server must remain
single-threaded (excepting CGI), should never exit unexpectedly, and must remain
nonblocking.

# Instructions

This project, like most projects at 42, can be compiled using the provided
makefile:
```sh
make
```
> [!NOTE]
>Compilation has only been tested using clang++ (>=15), but should not require much
>configuration to work with your local c++ compiler.


Further information on how to run the program can be found using `./webserv -h`.

There is a fully commented configuration file in the `example` directory that
can be used as a reference.

For CGI to function as expected, you must have `/usr/bin/env` and the `timeout`
utility installed.
Any script that is executable from the get-go should work, including:
- script files with shebangs (`#!/path/to/interpreter`)
- binary executables
> [!NOTE]
>The server will not be able to execute these files without execution
>permissions!

In addition to this, it supports filename extension based interpreting for the
following extensions:
- haskell (`.hs`)
- python (`.py`)
- bash/sh (`.sh`)
- cgi-binaries (`.cgi`)
> [!NOTE]
>cgi-binaries are not ran using any interpreter, they are just a special file
>extension that the server will interpret as a cgi script

# Resources

- [HTTP/1.1 specification](https://www.rfc-editor.org/rfc/rfc9112), related documents hyperlinked within
- [CGI/1.1 specification](https://www.rfc-editor.org/rfc/rfc3875), related documents hyperlinked within
- various manual pages

AI was not used at any point during the creation of this project.

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
- Set-Cookies field from CGI can and does just work, arguably not the servers
  responibility to handle cookies. static pages dont have a way to manipulate
  header fields, and thus it is the CGI's responsibility to change cookies and
  alter behaviour accordingly (or clients)
- www/cgi-bin/haskell changes behaviour based on cookies
### multiple cgi types
#### done
- can currently run:
    - binary
    - haskell
    - python
    - shell
    - .cgi (anything that can run directly)
- can read http bodies over stdin now
- theres an environment
- the little that needs to be parsed by the server is now being parsed by the server
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
- implemented routes
