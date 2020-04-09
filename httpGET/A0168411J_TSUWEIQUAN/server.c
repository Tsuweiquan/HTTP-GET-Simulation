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
#include <sys/time.h>

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
    int fd, sendfd;
    int nbytes, content_len, sent_bytes;
    char data[PKT_SIZE];
    char responseMSG[PKT_SIZE];
    bool isGET = false;
    bool isPersistent = false;
    off_t offset;
    int newfd[30] = {0}; // 30 client max
    int next = 0;
    int max_client = 30;
    int max_sd, sd;
    int downloadObjectsCounter = 0;
    fd_set readfds;

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

    //listen to the socket, accepting 2 connection to the master socket
    if (listen(fd, 10) < 0)
    {
        perror("Listen failure\n");
        exit(1);
    }

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        max_sd = fd;
        // initialize all other newfds
        // if there is new connection that i have set in the previous loop, i set it now
        // getting the max_sd
        for (int i = 0; i < max_client; i++)
        {
            sd = newfd[i];
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }
        if (select(max_sd + 1, &readfds, 0, 0, 0) < 0) {// no blocking
            perror("select error\n");
            exit(-1);
        }
       
        if (FD_ISSET(fd, &readfds))
        {
            if (next == max_client) next = 0;
            newfd[next++] = accept(fd, (struct sockaddr *)&client, &sockaddr_len);
        }   
        
        for (int i = 0; i < max_client; i++)
        {
            if (FD_ISSET(newfd[i], &readfds))
            {
                printf("New client connected from port number %d and IP address %s \n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
                if (recv(newfd[i], data, PKT_SIZE, 0) < 0)
                {
                    printf("Receiving error, data length < 0 \n");
                    exit(-1);
                }
                printf("============== Recv HTTP GET Request ==============\n");
                // printf("%s\n", data);
                //GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: keep-alive\r\n\r\n
                char *copyData = malloc(PKT_SIZE);
                strcpy(copyData, data);
                // Check if it's a GET request
                char *pktType = strtok(data, ": ");
                if (strcmp(pktType, "GET") == 0)
                {
                    printf("GET Request Packet Received!\n");
                    isGET = true;
                }
                
                // Check Connection Type. keep-alive or closed
                if (strstr(copyData, "keep-alive") != NULL) {
                    printf("Requesting Persistent Connecton!\n");
                    isPersistent = true;
                } else if (strstr(copyData, "closed") != NULL) {
                    printf("Requesting Non Persistent Connecton!\n");
                    isPersistent = false;
                }

                // Check Request Type (index.html or a.jpg or b.mp3 or c.txt)
                int requestItem = 0;
                if (strstr(copyData, "index.html") != NULL)
                {
                    requestItem = 0;
                }
                else if (strstr(copyData, "a.jpg") != NULL)
                {
                    requestItem = 1;
                }
                else if (strstr(copyData, "b.mp3") != NULL)
                {
                    requestItem = 2;
                }
                else if (strstr(copyData, "c.txt") != NULL)
                {
                    requestItem = 3;
                }

                if (isPersistent == true && isGET == true)
                {
                    // Persistent Connection
                    if (requestItem == 0)
                    {
                        sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n");
                    }
                    else if (requestItem == 1)
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
                    else if (requestItem == 2)
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
                    else if (requestItem == 3)
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
                    if (nbytes = write(newfd[i], &responseMSG, sizeof(responseMSG)) != sizeof(responseMSG))
                    {
                        printf("Error Sending Reply\n");
                    }
                    printf("============== Send HTTP GET Response ==============\n");
                    printf("%s\n", responseMSG);
                    // now send the file
                    if (requestItem != 0)
                    {
                        offset = 0;
                        /* Sending file data */
                        while (((sent_bytes = sendfile(newfd[i], sendfd, &offset, BUFSIZ)) > 0) && (content_len > 0))
                        {
                            content_len -= sent_bytes;
                            printf("Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, content_len);
                        }
                        downloadObjectsCounter++;
                        printf("All of object %d chunks sent!\n", requestItem);
                    }
                    if (downloadObjectsCounter == TOTAL_OBJECTS)
                    {
                        // check if client close his socket already
                        if (recv(newfd[i], data, PKT_SIZE, 0) <= 0) {
                            printf("Client has closed his socket. Proceeding to close my socket now.\n");
                            close(newfd[i]);
                            FD_CLR(newfd[i], &readfds);
                            newfd[i] = 0;
                            downloadObjectsCounter = 0;
                        }
                    }
                }
                else if (isPersistent == false && isGET == true)
                {   // Non persistent. Need to close socket after every packet
                    if (requestItem == 0)
                    {
                        sprintf(responseMSG, "HTTP/1.1 200 OK\r\nDate: Mon, 06 Apr 2020 12:28:53 GMT\r\nServer: Apache/2.2.14 (Win32)\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nETag: \"34aa387-d-1568eb00\"\r\nVary: Authorization,Accept\r\nAccept-Ranges: bytes\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: closed\r\n\r\n");
                    }
                    else if (requestItem == 1)
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
                    else if (requestItem == 2)
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
                    else if (requestItem == 3)
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
                    if (nbytes = write(newfd[i], &responseMSG, sizeof(responseMSG)) != sizeof(responseMSG))
                    {
                        printf("Error Sending Reply\n");
                    }
                    printf("============== Send HTTP GET Response ==============\n");
                    printf("%s\n", responseMSG);
                    // now send the file
                    if (requestItem != 0)
                    {
                        offset = 0;
                        /* Sending file data */
                        while (((sent_bytes = sendfile(newfd[i], sendfd, &offset, BUFSIZ)) > 0) && (content_len > 0))
                        {
                            content_len -= sent_bytes;
                            printf("Server sent %d bytes from file's data, offset is now : %ld and remaining data = %d\n", sent_bytes, offset, content_len);
                        }
                        printf("All of object %d chunks sent!\n", requestItem);
                        // check if client close his socket already
                        if (recv(newfd[i], data, PKT_SIZE, 0) <= 0) {
                            printf("Client has closed his socket. Proceeding to close my socket now.\n");
                            close(newfd[i]);
                            newfd[i] = 0;
                            FD_CLR(newfd[i], &readfds);
                        }
                    }
                    else 
                    {
                        // check if client close his socket already
                        if (recv(newfd[i], data, PKT_SIZE, 0) <= 0) {
                            printf("Client has closed his socket. Proceeding to close my socket now.\n");
                            close(newfd[i]);
                            newfd[i] = 0;
                            FD_CLR(newfd[i], &readfds);
                        }
                    }
                }
            }
        }
    }
}