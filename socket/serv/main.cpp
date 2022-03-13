#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "profile.h"
#include "server.h"

const std::string null_string = "\0";
int connections[MAXCLIENT];
int connect_count = 0;//число подключенных клиентов
int disconnect_count = 0;
char buf[SIZEOF_BUF];
profile profiles[MAXCLIENT];

void sendToAll( const char* buf, int index){
    for (int i = 0; i <= connect_count - 1; i++ ){
        if ( i != index && profiles[i].online == 1 )
            send(connections[i], buf, SIZEOF_BUF, 0);
    }
}

int sendPrivateMsg( std::string usrname, std::string msg, int sender_id){
    //maybe user made mistake
    if ( usrname == profiles[sender_id].user_name ){
        std::string error = "###You can't send private message yourself";
        send(sender_id, error.c_str(), SIZEOF_BUF, 0);
    }

    int rcv_id = findProfile(profiles, connect_count, usrname);
    if ( rcv_id == -1 || profiles[rcv_id].online == 0 ){
        std::string error= "###User is not online";
        send(sender_id, error.c_str(), SIZEOF_BUF, 0);
        return 0;
    }

    std::string system_msg= makePrivateMsg(profiles[rcv_id].user_name, profiles[sender_id].user_name, msg);
    send(connections[rcv_id], system_msg.c_str(), SIZEOF_BUF, 0);
    std::cout << system_msg << std::endl;
    return 1;

}

void disconnect( int user_d, std::string username, int index ){
    //shutting down connection and deletting deskriptor
    shutdown(user_d, 2);
    close(user_d);

    //making string to inform other users about disconnecting
    if ( !profiles[index].user_name.empty() ) {
        std::string temp = "*** ";
        temp += username;
        temp += " is diconnected";
        sendToAll(temp.c_str(), user_d);
    }
    //inform administrator of serever about disconnecting
    if ( !profiles[index].user_name.empty() )
        std::cout << profiles[index].user_name << " is disconnected" << std::endl;
    else
        std::cout << "New user" << " is disconnected" << std::endl;

    //preparing to reform the structure of data
    profiles[index].online = 0;
    profiles[index].user_name.clear();
    profiles[index].privates.clear();
    connections[index] = -1;
    disconnect_count++;
}

void clientHandler( char* buf, int user_d, int index ) {
    //We have to change buf, becuase we need string
    mapIntoStdSring(buf);
    processingStr(buf);

    //Before processing buffer can be useless
    if ( buf[0] == 0 ) {
        std::string error = "###You've sent empty string";
        send(user_d, error.c_str(), SIZEOF_BUF, 0);
        return;
    }

    //First, user make his name
    if ( profiles[index].user_name.empty() ){
        if ( correctName(buf) ){
            std::string temp_1 = buf;
            if ( nameIsNotUsed(temp_1, profiles, MAXCLIENT) ) {
                profiles[index].user_name = temp_1 + null_string;
                std::cout << "Name of new user: " << profiles[index].user_name << std::endl;

                std::string msg = "** New User: ";
                std::string temp_2 = msg + profiles[index].user_name;
                sendToAll(temp_2.c_str(), index);

                std::string congat = "### Glad to see you, ";
                congat += profiles[index].user_name;
                send(user_d, congat.c_str(), SIZEOF_BUF, 0);

                profiles[index].online = 1;
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
            for (int i = 0; i < connect_count; i++) {
                if ( profiles[i].online == 1 ) {
                    std::string temp_3 = ">";
                    temp_3 += profiles[i].user_name;
                    if (i == index)
                        temp_3 += " (its you!)";
                    send(user_d, temp_3.c_str(), SIZEOF_BUF, 0);
                }
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
            if ( sendPrivateMsg(usrname, msg, index) ) {
                profiles[index].privates.insert(usrname);
            }
        }
        else if( temp_1 == "\\privates"){
            //just send privates
            std::string msg_1 = "You've sent private msg to this users:";
            send(user_d, msg_1.c_str(), SIZEOF_BUF, 0);
            for (const auto& private_profile : profiles[index].privates) {
                std::string t = ">";
                t += private_profile;
                send(user_d, t.c_str(), SIZEOF_BUF, 0);
            }
        }
        else if( temp_1.substr(0, 5) == "\\quit"){
            // quit from server
            if ( temp_1.size() > 5 ) {
                std::string message = temp_1.substr(6, temp_1.size() - 6);

                std::string temp_2 = "[";
                temp_2 += "farewell message from ";
                temp_2 += profiles[index].user_name;
                temp_2 += "]: ";
                temp_2 += message;

                sendToAll(temp_2.c_str(), index);
            }

            disconnect(user_d, profiles[index].user_name, index);
        }
        else{
            //command is not added or not correct
            std::string non_excisting= "Command doesn't exsist";
            send(user_d, non_excisting.c_str(), SIZEOF_BUF, 0);
        }
    }
    else{
        //just send message to all
        std::string msg = profiles[index].make_usr_name(buf);
        const char* str = msg.c_str();
        sendToAll(str, index);
        std::cout << str << std::endl;
    }

}


int main() {
    initConnections(connections, MAXCLIENT);


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

        //reform!


        for(int i = 1; i <= connect_count; i++ ){
            if ( connections[i-1] != -1 ) {
                FD_SET(connections[i - 1], &readfds);
                if (connections[i - 1] > max_d)
                    max_d = connections[i - 1];
            }
        }

        int select_result =  select(max_d+1, &readfds, 0, 0, 0);

        //Error is select()
        if ( select_result < 1 ){
            std::cout << "Error in select(): " << strerror(errno) << std::endl;
            exit(1);
        }

        //Message from listen. Maybe there is new connection
        if ( FD_ISSET(sListen, &readfds) && (connect_count <= MAXCLIENT ) ){
            int new_connection = accept(sListen, (struct sockaddr *) (&addr), (socklen_t *) &size_of_addr);
            if (new_connection == 0) {
                std::cout << "Error in connecting" << std::endl;
            } else {
                std::cout << "New client is connected." << std::endl;
            }
            std::string hello= "### Welcome to Arthur's server. Write your username";
            send(new_connection, hello.c_str(), SIZEOF_BUF, 0);

            connect_count++;
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

        //Check our clients, somebody wants to send something for us
        for(int i = 1; i <= connect_count; i++ ){
            if ( connections[i-1] != -1 ) {
                if (FD_ISSET(connections[i - 1], &readfds)) {
                    buf[0] = 0;
                    int index = i - 1;

                    int boolean = recv(connections[i - 1], buf, SIZEOF_BUF, 0);
                    if (boolean == 0) {
                        disconnect(connections[index], profiles[index].user_name, index);
                        continue;
                    }

                    if (!consistingEnd(buf)) {
                        std::string error = "### Error: try to send shorter message";
                        send(connections[i - 1], error.c_str(), SIZEOF_BUF, 0);
                        continue;
                    }

                    clientHandler(buf, connections[i - 1], index);
                }
            }
        }
    }
}