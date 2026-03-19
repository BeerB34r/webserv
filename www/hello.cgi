#!/usr/bin/env sh

fatal() {
	echo "[FATAL]" $@
	exit 1
}

echo -n "\r\n"
echo -n "Hello, CGI!\n"
