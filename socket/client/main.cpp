#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <string>

#define MAXCLIENT 100
#define MINIMAL_SIZEOF_BUF 50
#define SIZEOF_MESSAGE 206
#define SIZEOF_BUF MINIMAL_SIZEOF_BUF+SIZEOF_MESSAGE
#define STDINPUT 0

char buf[SIZEOF_BUF];

int main() {
    struct sockaddr_in addr = sockaddr_in{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1024) ;
    addr.sin_family = AF_INET;

    int connection =  socket(AF_INET, SOCK_STREAM, 0);
    if ( connect( connection, (struct sockaddr*) &addr, sizeof(addr)) != 0 ){
        std::cout << "Error in connection: " << strerror(errno) << std::endl;
        exit(1);
    }
    std::cout << "You're connected" << std::endl;
  //  std::cout << buf << std::endl;

    for(;;){
        fd_set readfds;

        int max_d = connection;

        FD_ZERO(&readfds);
        FD_SET(connection, &readfds);

        FD_SET(STDINPUT, &readfds);

        int select_result =  select(max_d+1, &readfds, 0, 0, 0);

        if ( FD_ISSET(connection, &readfds) ){
            int boolean = recv(connection, buf, SIZEOF_BUF, 0);
            if ( boolean == 0 ) {
                std::cout << "You're disconnected" << std::endl;                    // disconnect
                exit(0);
            }
            std::cout << buf << std::endl;
        }

        if (FD_ISSET(STDINPUT, &readfds)){
            read(STDINPUT, buf, SIZEOF_BUF);
            send(connection, buf, SIZEOF_BUF, 0);
        }
    }

    return 0;
}