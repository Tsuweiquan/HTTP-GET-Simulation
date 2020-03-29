

main () {
    int fd, connfd, sockfd;
    int next = 0;
    int newfd[10];  // set of sockets, at most 10 ACTIVE connections
    fd_set readfds, allfds; 
    
    // create a socket
    fd = socket();
    // bind the socket
    bind(fd,...); 
    // listen to the socket
    listen(fd,...);

    // zero all my sockets that is currently open
    FD_ZERO(&allfds);
    // only 1 socket open above so i set it to 1
    FD_SET(fd, &allfds);

    while(1){
        // copy the fds vector
        readfds = allfds
        // everytime select is called, it will clear and only the one is ready will be set to 1
        if (select(maxfd + 1, &readfds, 0, 0, 0) < 0){
            perror("select");
            exit(1);
        }
        
        // if set means new connection has come in and should be handled.
        if(FD_ISSET(fd, &readfds)){

            connfd = accept(fds,...);
            newfd[next] = connfd;
            next += 1;

            FD_SET(connfd, &allfds)
        }
        for (all entries in newfd[]){
            sockfd = newfd[i];
            if( FD_ISSET(sockfd, &readfds)){
                //handle connection here. R/W
            }
        }
    }

}