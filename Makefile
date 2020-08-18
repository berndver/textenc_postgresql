all: textenc.so server

textenc.so: textenc.c
			gcc -shared -o textenc.so textenc.c -I /usr/include/postgresql/12/server/ 

server: server.c
			gcc -o server server.c