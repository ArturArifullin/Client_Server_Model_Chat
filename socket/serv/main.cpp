#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdlib>
#include <sys/time.h>
#include <cstring>
#include <unistd.h>
#include "profile.h"
#include "server.h"

const std::string null_string = "\0";
int connections[MAXCLIENT]= {0};
int Count = 0; //число подключенных клиентов
char buf[SIZEOF_BUF];
profile profiles[MAXCLIENT];

void sendToAll( const char* buf, int id){
    for ( int i = 0; i <= Count - 1; i++){
        if ( i != id)
            send(connections[i], buf, SIZEOF_BUF, 0);
    }
}

int sendPrivateMsg( std::string usrname, std::string msg, int sender_id){
    //maybe user made mistake
    if ( usrname == profiles[sender_id].user_name ){
        std::string error = "###You can't send private message yourself";
        send(sender_id, error.c_str(), SIZEOF_BUF, 0);
    }

    int rcv_id = findProfile(profiles, Count, usrname);
    if ( rcv_id == -1 ){
        std::string error= "###User is not online";
        send(sender_id, error.c_str(), SIZEOF_BUF, 0);
        return 0;
    }

    std::string system_msg= makePrivateMsg(profiles[rcv_id].user_name, profiles[sender_id].user_name, msg);
    send(connections[rcv_id], system_msg.c_str(), SIZEOF_BUF, 0);
    std::cout << system_msg << std::endl;
    return 1;

}

//void sendPrivateMsg()

int lengthOfFirtsWord(const char* buf){
    int i;
    for(i = 0; !(buf[i] == 0 || buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\r'); i++){
    }
    return i;
}

void clientHandler( char* buf, int user_d, int id ) {
    //We have to change buf
    mapIntoStdSring(buf);
    processingStr(buf);

    //Before processing buffer can be useless
    if ( buf[0] == 0 ) {
        std::string error = "###You've sent empty string";
        send(user_d, error.c_str(), SIZEOF_BUF, 0);
        return;
    }

    //First, user make his name
    if ( profiles[id].user_name.empty() ){
        if ( correctName(buf) ){
            std::string temp_1 = buf;
            if ( nameIsNotUsed(temp_1, profiles, MAXCLIENT) ) {
                profiles[id].user_name = temp_1 + null_string;
                std::cout << "Name of new user: " << profiles[id].user_name << ". His id: " << id << std::endl;

                std::string msg = "** New User: ";
                std::string temp_2 = msg + profiles[id].user_name;
                sendToAll(temp_2.c_str(), id);

                std::string congat = "### Glad to see you, ";
                congat += profiles[id].user_name;
                send(user_d, congat.c_str(), SIZEOF_BUF, 0);
            }
            else{
                std::string msg = "###Name is used by other user of chat";
                send(user_d, msg.c_str(), SIZEOF_BUF, 0);
            }
        }
        else{
            std::string msg = "###Name is not correct";
            send(user_d, msg.c_str(), SIZEOF_BUF, 0);
        }
    }
    else if( buf[0] == '\\' ) { //Check commands, '\' means that user wanna use commands
        std::string temp_1 = buf;

        if ( temp_1 == "\\users"){
            std::string temp_2 = "** List of online users (the names are listed via <): ";
            send(user_d, temp_2.c_str(), SIZEOF_BUF, 0);
            for ( int i = 0; i < Count; i++) {
                std::string temp_3 = ">";
                temp_3 +=  profiles[i].user_name;
                if ( i == id )
                    temp_3 += " (its you!)";
                send(user_d, temp_3.c_str(), SIZEOF_BUF, 0);
            }

        }
        else if ( temp_1 == "\\help"){
            std::string temp_2 = "** You can you following commands: ";
            temp_2 += "\\help, \\users, \\private <nickname> <message>, \\privates,  \\quit <message>";
            send(user_d, temp_2.c_str(), SIZEOF_BUF, 0);
        }
        else if ( (temp_1.substr(0, 8) == "\\private") &&  (temp_1 != "\\privates" ) ) { //sending private message
            if ( (spaceNum( temp_1.c_str() ) < 2) || temp_1[8] != ' ' ){ //try to check correct form of \private
                std::string error_private = "###Not correct attempt to send private message. Try again";
                send(user_d, error_private.c_str(), SIZEOF_BUF, 0);
                return;
            }

            //cut words for using
            int size = lengthOfFirtsWord(temp_1.substr(9, temp_1.size() - 9).c_str());
            std::string usrname = temp_1.substr(9, size);
            std::string msg = temp_1.substr(10 + size, temp_1.size() - 10 - size);

            //sending
            if ( sendPrivateMsg(usrname, msg, id) ) {
                profiles[id].privates.insert(usrname);
            }
        }
        else if( temp_1 == "\\privates"){
            //just send privates
            std::string msg_1 = "You've sent private msg to this users:";
            send(user_d, msg_1.c_str(), SIZEOF_BUF, 0);
            for (const auto& private_profile : profiles[id].privates) {
                std::string t = ">";
                t += private_profile;
                send(user_d, t.c_str(), SIZEOF_BUF, 0);
            }
        }
        else if( temp_1 == "\\quit"){
            //quit from server
        }
        else{
            //command is not added or not correct
            std::string non_excisting= "Command doesn't exsist";
            send(user_d, non_excisting.c_str(), SIZEOF_BUF, 0);
        }
    }
    else{
        //just send message to all
        std::string msg = profiles[id].make_usr_name(buf);
        const char* str = msg.c_str();
        sendToAll(str, id);
        std::cout << str << std::endl;
    }

}


int main() {
    struct sockaddr_in addr = sockaddr_in{};
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1024);
    addr.sin_family = AF_INET;
    int size_of_addr = sizeof(addr); //for using on accept() function

    int sListen = socket(AF_INET, SOCK_STREAM, 0); //create socket
    int res = bind(sListen, (struct sockaddr *)(&addr), sizeof(addr));
    if ( listen( sListen, SOMAXCONN) == -1 ){
        std::cout << "Error in listening" << std::endl;
        exit(1);
    }

    while(1){
        fd_set readfds;
        int max_d = sListen;
        FD_ZERO(&readfds);
        FD_SET(sListen, &readfds);
        FD_SET(STDINPUT, &readfds);

        for( int i = 1; i <= Count; i++ ){
            FD_SET(connections[i-1], &readfds);
            if ( connections[i-1] > max_d )
                max_d = connections[i-1];
        }

        int select_result =  select(max_d+1, &readfds, 0, 0, 0);

        //Error is select()
        if ( select_result < 1 ){
            std::cout << "Error in select(): " << strerror(errno) << std::endl;
            exit(1);
        }

        //Message from listen. Maybe there is new connection
        if ( FD_ISSET(sListen, &readfds) && ( Count <= MAXCLIENT ) ){
            int new_connection = accept(sListen, (struct sockaddr *) (&addr), (socklen_t *) &size_of_addr);
            if (new_connection == 0) {
                std::cout << "Error in connecting" << std::endl;
            } else {
                std::cout << "New client is connected. His id: "<< Count << std::endl;
            }
            std::string hello= "### Welcome to Arthur's server. Write your username";
            send(new_connection, hello.c_str(), SIZEOF_BUF, 0);

            Count++;
            connections[Count-1] = new_connection; //!!!! Work about it
        }

        //Server admin tries to do something
        if (FD_ISSET(STDINPUT, &readfds)){
            read(STDINPUT, buf, SIZEOF_BUF);
            std::cout << buf << std::endl;
        }

        //Check our clients, somebody wants to send something for us
        for( int i = 1; i <= Count; i++ ){
            if( FD_ISSET(connections[i-1], &readfds) ){
                buf[0] = 0;
                recv(connections[i-1], buf, SIZEOF_BUF, 0);

                if( !consistingEnd(buf) ){
                    std::string error = "### Error: try to send shorter message";
                    send( connections[i-1], error.c_str(), SIZEOF_BUF, 0 );
                    continue;
                }

                //Each client is bound with him unic socket, number of socket is id
                int id= i-1;
                clientHandler(buf, connections[i-1], id);
            }
        }
    }
}