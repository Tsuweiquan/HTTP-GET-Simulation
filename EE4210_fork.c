main() {
    int fd;
    struct sock_addr_in;
    fd = socket();
    bind(fd, ...);
    listen(fd,...);
    while(1) {
        connfd = accept(fd,...);
        pid = fork();
        if (pid > 0) {
            // parent
            // no need to do anything with connfd to client
            //  because child process will handle it
            close(connfd);
        }
        if (pid == 0){
            // child
            // fd is the socket for 3 way handshake
            // child only need to handle the connection using connfd
            // hence can close fd
            close(fd); //child no need worry about new connections coming in
            // handle connection
            recv();
            send()
            close(connfd);
            exit(0);
        }
        if (pid < 0){
            // error
        }
        /* handle connection */
        recv();
        send();
        close(connfd);
    }
}