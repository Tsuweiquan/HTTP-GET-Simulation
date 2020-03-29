#include<stdio.h>
#include<stdlib.h>
#include<winSock2.h>
// #include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<error.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

#define MAX_DATA 1024

void main(int argc, char **argv) {
    struct sockaddr_in server;
    int sock;
    char input[MAX_DATA];
    char output[MAX_DATA];

    if(argc != 3) {
        printf("Too few arguments \n");
		printf("Usage: %s <IP address> <port number> \n", argv[0]);
        printf("Example: ./client_non_persistent 192.168.1.233 80");
		exit(1);
    }

    // create a TCP socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("server socket: ");
		exit(1);
	}

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argc[1);
    bzero(&server.sin_zero, 8);

    // Persistent. Establish 3 way handshake by connect once only
    if((connect(sock, (struct sockaddr*) *server), sizeof(server)) < 0){
        perror("Connection to server failed");
        exit(1);
    }
    printf("TCP connection completed!");
        

    while(1) {    


    }
}