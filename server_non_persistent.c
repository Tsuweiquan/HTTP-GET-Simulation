#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<stdint.h>
// #include<WinSock2.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<error.h>
#include<strings.h>
#include<unistd.h>
#include<arpa/inet.h>

typedef struct Packet {
    uint16_t pktType;            // 1 --> Request Pkt || 2 --> Reply Pkt
    uint16_t connectionType;     // 0 --> nonPersistent || 1 --> Persistent
    uint16_t numObjects;         // numberOfObjects to be downloaded
} Request_Packet;

void main(int argc, char **argv) {
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sockaddr_len = sizeof(struct sockaddr_in);
    int fd, newfd;
    int data_len;
    int nbytes;
    // char data[MAX_DATA];

    if(argc != 2) {
		printf("Too few arguments \n");
		printf("Usage: %s <port number> \n", argv[0]);
        printf("Example: ./server 80");
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
        perror("Bind failure\n");
        exit(1);
    }

    //listen to the socket, accepting 2 connection
    if(listen(fd, 2) < 0) {
        perror("Listen failure\n");
        exit(1);
    }

    newfd = accept(fd, (struct sockaddr*) &client, &sockaddr_len);
    if (newfd < 0) {
        perror("Create New Socket failure :");
        exit(1);
    }

    printf("New client connected from port number %d and IP address %s \n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
    Request_Packet recv_packet;
    if(recv(newfd, (void *)&recv_packet, sizeof(Request_Packet), 0) < 0) {
        printf("Receiving error, data length < 0 \n");
        exit(-1);
    }

    uint16_t pktType = ntohs(recv_packet.pktType);
    uint16_t connectionType = ntohs(recv_packet.connectionType);
    uint16_t numObjects = ntohs(recv_packet.numObjects);
    
    if (connectionType == 0) { // 0 --> Non Persistent
        printf("Request Packet received from Client.\n");
        printf("Received pktType = %"PRIu16"\n", pktType );
        printf("Received connectionType = %"PRIu16"\n", connectionType );
        printf("Received numObjects = %"PRIu16"\n", numObjects );
        Request_Packet replyPacket;
        replyPacket.pktType = htons(2);
        replyPacket.connectionType = htons(connectionType);
        replyPacket.numObjects = htons(3);
        if (nbytes = write(newfd, &replyPacket, sizeof(Request_Packet)) != sizeof(Request_Packet)) {
            printf("Error Sending Reply\n");
        }
        printf("Sent Reply Packet, informing client there is 3 objects to be downloaded.\n");
        close(fd);
        close(newfd);

        int totalObjects = 3;
        for (int i = 1; i <= totalObjects; i++){
            // for each object, recreate the socket to establish 3 way handshake
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
                perror("Bind failure\n");
                exit(1);
            }

            //listen to the socket, accepting 2 connection
            if(listen(fd, 2) < 0) {
                perror("Listen failure\n");
                exit(1);
            }

            newfd = accept(fd, (struct sockaddr*) &client, &sockaddr_len);
            if (newfd < 0) {
                perror("Create New Socket failure :\n");
                exit(1);
            }

            Request_Packet recv_packet;
            if(recv(newfd, (void *)&recv_packet, sizeof(Request_Packet), 0) < 0) {
                printf("Receiving object %d request packet corrupted. \n", i);
                exit(-1);
            }
            uint16_t pktType = ntohs(recv_packet.pktType);
            uint16_t numObjects = ntohs(recv_packet.numObjects);
            if (pktType == 1 && numObjects == i && i == 1) {  
                // request packet and requesting for Object 1
                // send the first object ()
                //sendfile(sockfd, filefd, NULL, BUFSIZE);
                printf("Ready to send file %d from server.\n", i);
            }else if (pktType == 1 && numObjects == i && i == 2) {  
                // request packet and requesting for Object 1
                // send the first object ()
                //sendfile(sockfd, filefd, NULL, BUFSIZE);
                printf("Ready to send file %d from server.\n", i);
            }else if (pktType == 1 && numObjects == i && i == 3) {  
                // request packet and requesting for Object 1
                // send the first object ()
                //sendfile(sockfd, filefd, NULL, BUFSIZE);
                printf("Ready to send file %d from server.\n", i);
            }
            close(fd);
            close(newfd);
        }


    } else if (connectionType == 2) {
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

// https://stackoverflow.com/questions/15445207/sending-image-jpeg-through-socket-in-c-linux
// https://stackoverflow.com/questions/30582473/send-mp3-file-over-socket-in-c
// https://www.geeksforgeeks.org/c-program-for-file-transfer-using-udp/
// https://stackoverflow.com/questions/13215656/c-sendfile-and-send-difference