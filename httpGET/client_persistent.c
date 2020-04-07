#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PKT_SIZE 1500
#define IS_PERSISTENT "keep-alive"
#define FILE_1 "a_received_persistent.jpg"
#define FILE_2 "b_received_persistent.mp3"
#define FILE_3 "c_received_persistent.txt"

void main(int argc, char **argv)
{
    struct sockaddr_in server;
    int sock, nbytes, incomingFileSize, len;
    char requestMSG[PKT_SIZE];
    char recvData[PKT_SIZE];
    char fileBuffer[BUFSIZ];
    bool requestSuccess = false;
    int numberOfObjects;
    FILE *received_file;

    if (argc != 3)
    {
        printf("Too few arguments \n");
        printf("Usage: %s <IP address> <port number> \n", argv[0]);
        printf("Example: ./client_non_persistent 192.168.1.233 80\n");
        exit(1);
    }

    // create a TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("server socket: ");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&server.sin_zero, 8);

    // Non Persistent. Always establish 3 way handshake by connect
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connection to server failed\n");
        exit(1);
    }
    printf("TCP connection completed!\n");

    // creating the HTTP Request Packet to send to server
    sprintf(requestMSG, "GET /index.html HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\n\r\n", argv[1], IS_PERSISTENT);
    printf("============== Send HTTP GET Request ==============\n");
    printf("%s\n", requestMSG);

    if (nbytes = write(sock, &requestMSG, sizeof(requestMSG)) != sizeof(requestMSG))
    {
        printf("error sending my HTTP request packet\n");
        exit(-1);
    }
    printf("Sent HTTP Request Packet.\n");

    if (recv(sock, &recvData, sizeof(recvData), 0) < 0)
    {
        printf("Receiving error, data length < 0 \n");
        exit(-1);
    }
    printf("============== Recv HTTP GET Response ==============\n");
    printf("%s\n", recvData);
    printf("Received HTTP Response Packet.\n");
    // Check if status 200
    char *pktType = strtok(recvData, " ");
    while (pktType != NULL)
    {
        if (strcmp(pktType, "200") == 0)
        {
            printf("Status 200, Request Success.\n");
            requestSuccess = true;
        }
        pktType = strtok(NULL, " ");
    }

    // After reading the HTML file, the client knows that it needs to download 3 objects
    numberOfObjects = 3;

    if (requestSuccess == true)
    {
        for (int i = 0; i < numberOfObjects; i++)
        {
            // Since Persistent, socket is already open from TCP connection. Resuse the same socket
            if (i == 0)
            {
                sprintf(requestMSG, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\n\r\n", FILE_1, argv[1], IS_PERSISTENT);
                received_file = fopen(FILE_1, "w");
            }
            else if (i == 1)
            {
                sprintf(requestMSG, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\n\r\n", FILE_2, argv[1], IS_PERSISTENT);
                received_file = fopen(FILE_2, "w");
            }
            else
            {
                sprintf(requestMSG, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: %s\r\n\r\n", FILE_3, argv[1], IS_PERSISTENT);
                received_file = fopen(FILE_3, "w");
            }
            printf("============== Send HTTP GET Request ==============\n");
            printf("%s", requestMSG);
            // Send GET Request to server
            if (nbytes = write(sock, &requestMSG, sizeof(requestMSG)) != sizeof(requestMSG))
            {
                printf("error sending my HTTP request packet\n");
                exit(-1);
            }
            printf("Sent HTTP GET Request Packet for Object %d.\n", i + 1);

            // Received GET Response from Server
            if (recv(sock, &recvData, sizeof(recvData), 0) < 0)
            {
                printf("Receiving error, data length < 0 \n");
                exit(-1);
            }
            printf("============== Recv HTTP GET Response ==============\n");
            printf("%s\n", recvData);
            
            // Extracting the number of Bytes the object is in the HTTP Response Packet
            char *pktType = strtok(recvData, "\r\n");
            while (pktType != NULL)
            {
                if (strcmp(pktType, "Accept-Ranges: bytes") == 0)
                {
                    pktType = strtok(NULL, "\r\n");
                    pktType = strtok(pktType, ": ");
                    pktType = strtok(NULL, ": ");
                    printf("%s\n", pktType);
                    break;
                }
                pktType = strtok(NULL, "\r\n");
            }
            incomingFileSize = atoi(pktType);
            printf("Expect file of size %d\n", incomingFileSize);
            if (received_file == NULL)
            {
                fprintf(stderr, "Failed to open file --> %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            // Received Object from Server
            while ((incomingFileSize > 0) && ((len = recv(sock, fileBuffer, BUFSIZ, 0)) > 0))
            {
                fwrite(fileBuffer, sizeof(char), len, received_file);
                incomingFileSize -= len;
                fprintf(stdout, "Receive %d bytes and Remaining:- %d bytes\n", len, incomingFileSize);
            }
            fclose(received_file);
        }
        // After all 3 objects are downloaded, proceed to close the socket.
        close(sock);
    }
    else
        exit(-1); // Initial HTTP request was not successful.
}