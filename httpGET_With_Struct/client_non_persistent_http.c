#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<stdint.h>
#include <sys/sendfile.h>
// #include<winSock2.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<error.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

void main(int argc, char **argv) {
    struct sockaddr_in server;
    int sock, nbytes;

    if(argc != 3) {
        printf("Too few arguments \n");
		printf("Usage: %s <IP address> <port number> \n", argv[0]);
        printf("Example: ./client_non_persistent 192.168.1.233 80\n");
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
    server.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&server.sin_zero, 8);

    // Non Persistent. Always establish 3 way handshake by connect
    if(connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
        perror("Connection to server failed\n");
        exit(1);
    }
    printf("TCP connection completed!\n");

    // creating the Request Packet to send to server
    char requestMSG[100]= {0};
    sprintf(requestMSG, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\n\r\n", ResourcePath, HostAddress, isPersistent);

    printf("Send HTTP Request Packet.\n");
    if (nbytes = write(sock, &p, sizeof(Request_Packet)) != sizeof(Request_Packet)) {
        printf("error writing my message");
    }
    
    Request_Packet recv_packet;
    if(recv(sock, (void *)&recv_packet, sizeof(Request_Packet), 0) < 0) {
        printf("Receiving error, data length < 0 \n");
        exit(-1);
    }
    uint16_t pktType = ntohs(recv_packet.pktType);
    uint16_t connectionType = ntohs(recv_packet.connectionType);
    uint16_t numObjects = ntohs(recv_packet.numObjects);
    printf("Received pktType = %"PRIu16"\n", pktType );
    printf("Received connectionType = %"PRIu16"\n", connectionType );
    printf("Received numObjects = %"PRIu16"\n", numObjects );
    printf("Received HTTP Response Packet from Server. There is %d objects to be downloaded.\n", numObjects);
    close(sock);
    
    int totalObjectsDownloaded = 0;
    for (int i = 1; i <= numObjects; i++ ) {
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("server socket: ");
            exit(1);
        }
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(argv[2]));
        server.sin_addr.s_addr = inet_addr(argv[1]);
        bzero(&server.sin_zero, 8);
        // Non Persistent. Always establish 3 way handshake by connect
        while(connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
            perror("Waiting for server to be Ready...\n");
        }
        printf("TCP connection completed!\n");
        // Send Request for Object i packet
        Request_Packet requestObject;
        requestObject.pktType = htons(1);
        requestObject.connectionType = htons(0);
        requestObject.numObjects = htons(i);
        if (nbytes = write(sock, &requestObject, sizeof(Request_Packet)) != sizeof(Request_Packet)) {
            printf("Request for Object %d failed.\n", i);
            exit(1);
        }
        printf("Ready to receive file %d from server.\n", i);
        close(sock);
        totalObjectsDownloaded++;
    }
    if(totalObjectsDownloaded == 3) {
        exit(0);
    }

}