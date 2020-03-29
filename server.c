#include<stdio.h>
#include<stdlib.h>
#include<WinSock2.h>
// #include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<error.h>
#include<strings.h>
#include<unistd.h>
#include<arpa/inet.h>


void main(int argc, char **argv) {
    struct sockaddr_in server;
    struct sockaddr_in client;
    int fd, newfd;

    if(argc != 2) {
		printf("Too few arguments \n");
		printf("Usage: %s <port number> \n", argv[0]);
        printf("Example: ./server 192.168.1.233 80")
		exit(1);
	}

    // TCP connection
    // create a socket for 3way handshake
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("server socket: ");
		exit(-1);
	}

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&server.sin_zero, 8);

    if(bind(fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        // if bind failed
        perror("Bind failure");
        exit(1);
    }

    //listen to the socket, accepting 2 connection
    if(listen(fd, 2) < 0) {
        perror("Listen failure");
        exit(1);
    }

    while(1) {
        newfd = accept(fd, (struct sockaddr*) &client, sizeof(client))
        if (newfd < 0) {
            perror("Create New Socket failure.");
            exit(1);
        }

        // read the request packet
        // if request is 0 --> HTTP request message
        // reply with ack 1
        // if request is 1 --> request for download of .jpg
        // reply with n ack for n objects ...
        // if request is 2 --> request for download of .mp3
        // reply with n ack for n objects ...
        // if request is 3 --> request for downlaod of .txt
        // reply with n ack for n objects ...
    }
}