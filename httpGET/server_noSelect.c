#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PKT_SIZE 1500
#define TOTAL_OBJECTS 3
#define FILE_1 "a.jpg"
#define FILE_2 "b.mp3"
#define FILE_3 "c.txt"

void main(int argc, char **argv)
{
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct stat file_stat;
    int sockaddr_len = sizeof(struct sockaddr_in);
    int fd, newfd, sendfd;
    int nbytes, content_len, sent_bytes;
    char data[PKT_SIZE];
    char responseMSG[PKT_SIZE];
    bool isGET = false;
    bool isPersistent = false;
    off_t offset;

    if (argc != 2)
    {
        printf("Too few arguments \n");
        printf("Usage: %s <port number> \n", argv[0]);
        printf("Example: ./server 80");
        exit(1);
    }

    // TCP connection
    // create a socket for 3way handshake
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("server socket: ");
        exit(-1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&server.sin_zero, 8);

    if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // if bind failed
        perror("Bind failure\n");
        exit(1);
    }

    //listen to the socket, accepting 2 connection
    if (listen(fd, 2) < 0)
    {
        perror("Listen failure\n");
        exit(1);
    }

    newfd = accept(fd, (struct sockaddr *)&client, &sockaddr_len);
    if (newfd < 0)
    {
        perror("Create New Socket failure :");
        exit(1);
    }

    printf("New client connected from port number %d and IP address %s \n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));

    if (recv(newfd, data, PKT_SIZE, 0) < 0)
    {
        printf("Receiving error, data length < 0 \n");
        exit(-1);
    }
    printf("============== Recv HTTP GET Request ==============\n");
    printf("%s\n", data);
    //GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\n\r\n

    // Check if it's a GET request
    char *pktType = strtok(data, ": ");
    if (strcmp(pktType, "GET") == 0)
    {
        printf("GET Request Packet Received!\n");
        isGET = true;
    }
    // Check Connection Type. keep-alive or closed
    while (pktType != NULL)
    {
        pktType = strtok(NULL, ": ");
        if (strcmp(pktType, "keep-alive\r\n\r\n") == 0)
        {
            printf("Requesting Persistent Connecton!\n");
            isPersistent = true;
            break;
        }
        else if (strcmp(pktType, "closed\r\n\r\n") == 0)
        {
            printf("Requesting non Persistent Connection!\n");
            isPersistent = false;
            break;
        }
    }

    // Send HTTP Response message, indicating 3 objects to be downloaded
    if (isPersistent == true)
    {
        sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n");
    }
    else
    {
        sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: closed\r\n\r\n");
    }
    printf("============== Send HTTP GET Response ==============\n");
    printf("%s\n", responseMSG);
    if (nbytes = write(newfd, &responseMSG, sizeof(responseMSG)) != sizeof(responseMSG))
    {
        printf("Error Sending Reply\n");
    }
    printf("Sent HTTP Response Packet with the html page.\n");
    close(fd);  // can close fb since both persistent/non-persistent does not need to use this.

    if (isPersistent == false)
    {   // non persistent connection
        close(newfd);
        for (int i = 0; i < TOTAL_OBJECTS; i++)
        {
            // create & open new socket
            if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
                perror("server socket: ");
                exit(-1);
            }

            server.sin_family = AF_INET;
            server.sin_port = htons(atoi(argv[1]));
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            bzero(&server.sin_zero, 8);

            if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                // if bind failed
                perror("Bind failure\n");
                exit(1);
            }

            //listen to the socket, accepting 2 connection
            if (listen(fd, 2) < 0)
            {
                perror("Listen failure\n");
                exit(1);
            }

            newfd = accept(fd, (struct sockaddr *)&client, &sockaddr_len);
            if (newfd < 0)
            {
                perror("Create New Socket failure :");
                exit(1);
            }

            if (recv(newfd, data, PKT_SIZE, 0) < 0)
            {
                printf("Receiving error, data length < 0 \n");
                exit(-1);
            }
            printf("Received packet from client. \n");
            printf("============== Recv HTTP GET Request ==============\n");
            printf("%s\n", data);

            if (i == 0)
            {
                sendfd = open(FILE_1, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: image/jpeg\r\nConnection: closed\r\n\r\n", content_len);
            }
            else if (i == 1)
            {
                sendfd = open(FILE_2, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: audio/mpeg\r\nConnection: closed\r\n\r\n", content_len);
            }
            else
            {
                sendfd = open(FILE_3, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: txt/html\r\nConnection: closed\r\n\r\n", content_len);
            }

            // send http response
            if (nbytes = write(newfd, &responseMSG, sizeof(responseMSG)) != sizeof(responseMSG))
            {
                printf("Error Sending Reply\n");
            }
            printf("============== Send HTTP GET Response ==============\n");
            printf("%s\n", responseMSG);
            printf("Sent HTTP Response Packet for Object %d.\n", i + 1);
            // now send the file
            offset = 0;
            /* Sending file data */
            while (((sent_bytes = sendfile(newfd, sendfd, &offset, BUFSIZ)) > 0) && (content_len > 0))
            {
                content_len -= sent_bytes;
                printf("Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, content_len);
            }
            close(fd);
            close(newfd);
        }
    }
    else
    { // Persistent Connection, newfd socket not closed.
        for (int i = 0; i < TOTAL_OBJECTS; i++)
        {
            if (recv(newfd, data, PKT_SIZE, 0) < 0)
            {
                printf("Receiving error, data length < 0 \n");
                exit(-1);
            }
            printf("============== Recv HTTP GET Request ==============\n");
            printf("%s\n", data);

            if (i == 0)
            {
                sendfd = open(FILE_1, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: image/jpeg\r\nConnection: keep-alive\r\n\r\n", content_len);
            }
            else if (i == 1)
            {
                sendfd = open(FILE_2, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: audio/mpeg\r\nConnection: keep-alive\r\n\r\n", content_len);
            }
            else
            {
                sendfd = open(FILE_3, O_RDONLY);
                if (sendfd == -1)
                {
                    fprintf(stderr, "Error opening file --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                /* Get file stats */
                if (fstat(sendfd, &file_stat) < 0)
                {
                    fprintf(stderr, "Error fstat --> %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                content_len = file_stat.st_size;
                sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nContent-Type: txt/html\r\nConnection: keep-alive\r\n\r\n", content_len);
            }

            // send http response
            if (nbytes = write(newfd, &responseMSG, sizeof(responseMSG)) != sizeof(responseMSG))
            {
                printf("Error Sending Reply\n");
            }
            printf("============== Send HTTP GET Response ==============\n");
            printf("%s\n", responseMSG);
            // now send the file
            offset = 0;
            /* Sending file data */
            while (((sent_bytes = sendfile(newfd, sendfd, &offset, BUFSIZ)) > 0) && (content_len > 0))
            {
                content_len -= sent_bytes;
                printf("Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, content_len);
            }
        }
        close(newfd);
    }
}

