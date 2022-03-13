#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include "profile.h"
#include "server.h"
#include "clienthandler.h"

int connections[MAXCLIENT]; // array of socket-descriptors
char buf[SIZEOF_BUF]; // buffer for send() and recv()
profile profiles[MAXCLIENT]; // profiles data base
int sListen; // listen socket

void handler_sigint_or_sigterm( int a ){
    std::string farewell = "### Server is turned off. Bye!";
    sendToAll(farewell.c_str(), -1, profiles, connections);

    for ( int i = 0; i < MAXCLIENT; i++ ) {
        if (connections[i] != -1) {
            shutdown(connections[i], 2);
            close(connections[i]);

            profiles[i].online = 0;
            profiles[i].user_name.clear();
            profiles[i].privates.clear();
            connections[i] = -1;
        }
    }

    shutdown(sListen, 2);
    close(sListen);

    exit(0);
}

int main() {
    initConnections(connections, MAXCLIENT);

    struct sockaddr_in addr = sockaddr_in{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1024);
    addr.sin_family = AF_INET;
    int size_of_addr = sizeof(addr); //for using on accept() function

    sListen = socket(AF_INET, SOCK_STREAM, 0); //create socket
    int res = bind(sListen, (struct sockaddr *)(&addr), sizeof(addr));
    if ( listen( sListen, SOMAXCONN) == -1 ){
        std::cout << "Error in listening" << std::endl;
        exit(1);
    }

    signal(SIGTERM, handler_sigint_or_sigterm);
    signal(SIGINT, handler_sigint_or_sigterm);
    while(1){
        fd_set readfds;
        int max_d = sListen;
        FD_ZERO(&readfds);
        FD_SET(sListen, &readfds);
        FD_SET(STDINPUT, &readfds);

        for(int i = 1; i <= MAXCLIENT; i++ ){
            if ( connections[i-1] != -1 ) {
                FD_SET(connections[i - 1], &readfds);
                if (connections[i - 1] > max_d)
                    max_d = connections[i - 1];
            }
        }

        int select_result =  select(max_d+1, &readfds, nullptr, nullptr, nullptr);

        //Error is select()
        if ( select_result < 1 ){
            std::cout << "Error in select(): " << strerror(errno) << std::endl;
            exit(1);
        }

        //Message from listen. Maybe there is new connection
        if ( FD_ISSET(sListen, &readfds) && (MAXCLIENT <= MAXCLIENT ) ){
            int new_connection = accept(sListen, (struct sockaddr *) (&addr), (socklen_t *) &size_of_addr);
            if (new_connection == 0) {
                std::cout << "Error in connecting" << std::endl;
            } else {
                std::cout << "New client is connected." << std::endl;
            }
            std::string hello= "### Welcome to Arthur's server. Write your username";
            send(new_connection, hello.c_str(), SIZEOF_BUF, 0);

            int temp = firstDisc(connections, MAXCLIENT);
            if ( temp == -1 ){
                std::cout << "I cant connect new usew" << std::endl;
            }
            else {
                connections[temp] = new_connection; //!!!! Work about it
            }
        }

        //Server admin tries to do something
        if (FD_ISSET(STDINPUT, &readfds)){
            read(STDINPUT, buf, SIZEOF_BUF);
            std::cout << buf << std::endl;
        }

        //Check our clients, somebody wants to send something to us
        for(int i = 0; i < MAXCLIENT; i++ ){
            if ( connections[i] != -1 ) {
                if (FD_ISSET(connections[i], &readfds)) {
                    buf[0] = 0;
                    int index = i;

                    int boolean = recv(connections[i], buf, SIZEOF_BUF, 0);
                    if (boolean == 0) {
                        disconnect(connections[index], profiles[index].user_name, index, profiles, connections);
                        continue;
                    }

                    if (!consistingEnd(buf)) {
                        std::string error = "### Error: try to send shorter message";
                        send(connections[i], error.c_str(), SIZEOF_BUF, 0);
                        continue;
                    }

                    clientHandler(buf, connections[i], index, profiles, connections);
                }
            }
        }
    }
}