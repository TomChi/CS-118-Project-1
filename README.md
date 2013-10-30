CS-118-Project-1
================

Basic web server in c serving MIME files over TCP connections.

Use gcc to compile. Ping server from a browser.

Requirements:
1. Print out received requests - DONE
2. Handle requests and extract requested file name - DONE
3. Retrieve requested resource and write to connection - DONE

For now, the server only serves HTML, JPEG and GIF files. Other file types are not supported. 
Tested with:
	test.html
	maru.jpg
	maru.gif

TODO: make the server more when handling unsupported types.