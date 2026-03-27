*This project has been created as part of the 42 curriculum by mde-beer*

Simple webserver in C++, written by [mde-beer](https://profile.intra.42.fr/users/mde-beer)/[BeerB34r](https://github.com/BeerB34r) during studies at [Codam/42 Amsterdam](https://codam.nl)

# Description
Webserv is the first real introduction to socket programming within the core
curriculum. The idea is to create a small nginx clone that is capable of serving
static websites as well as handling CGI scripts.
Amongst the formal restrictions are:
- the entire server must remain single-threaded (excepting CGI)
- the server should never exit unexpectedly
- the server must remain nonblocking

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
