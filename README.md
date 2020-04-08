
## EE4210 CA2 
There are 6 files with 2 folders.
- persistent/               (folder)
- nonpersistent/            (folder)
- client_persistent.c       (source code)
- client_non_persistent.c   (source code)
- server.c                  (source code)
- client_persistent         (executable)
- client_non_persistent     (executable)
- server                    (executable)
- a.jpg                     (file to be downloaded)
- b.mp3                     (file to be downloaded)
- c.txt                     (file to be downloaded)


=============== Instructions ===============
To compile source code
gcc server.c -o server
gcc client_non_persistent.c -o client_non_persistent
gcc client_persistent.c -o client_persistent

1) Run server
Params: ./server <port number>
Eg. ./server 8888

2) Run client_persistent
Params: /client_persistent <Ip Addr> <port number>
Eg. ./client_persistent 127.0.0.1 8888

3) Run client_non_persistent
Params: /client_non_persistent <Ip Addr> <port number>
Eg. ./client_non_persistent 127.0.0.1 8888

Upon running the client program, it will connect to the server. After the TCP connection is successful, the client will send HTTP GET
requests to the server to download the 3 files.
Running the client_persistent program will download the files into the persistent folder. 
Running the client_non_persistent program will download the files into the nonpersistent folder.   