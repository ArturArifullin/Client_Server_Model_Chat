#include "clienthandler.h"
#include "server.h"
#include <iostream>
#include <sys/socket.h>
#include <utility>
#include <unistd.h>

const std::string null_string = "\0"; // constant string for some mapping string

void sendToAll( const char* buf, int index, profile* profiles, int* connections){ // send to all users defined message
    for (int i = 0; i < MAXCLIENT; i++ ){
        if ( i != index && profiles[i].online == 1 )
            send(connections[i], buf, SIZEOF_BUF, 0);
    }
}

int sendPrivateMsg( const std::string& usrname, std::string msg, int sender_id,  profile* profiles, int* connections){ // send pricate message
    int rcv_id = findProfile(profiles, MAXCLIENT, usrname);
    //maybe user made mistake
    if ( rcv_id == -1 || profiles[rcv_id].online == 0 ){
        std::string error= "###User is not online";
        send(connections[sender_id], error.c_str(), SIZEOF_BUF, 0);
        return 0;
    }

    if ( profiles[rcv_id].user_name == profiles[sender_id].user_name ){
        std::string error = "###You can't send private message yourself";
        send(connections[sender_id], error.c_str(), SIZEOF_BUF, 0);
        return 0;
    }

    std::string system_msg= makePrivateMsg(profiles[rcv_id].user_name, profiles[sender_id].user_name, std::move(msg));
    send(connections[rcv_id], system_msg.c_str(), SIZEOF_BUF, 0);
    std::cout << system_msg << std::endl;
    return 1;
}

void disconnect( int user_d, std::string& username, int index,  profile* profiles, int* connections ){ //desconnecting
    //shutting down connection and deletting deskriptor
    shutdown(user_d, 2);
    close(user_d);

    //making string to inform other users about disconnecting
    if ( !profiles[index].user_name.empty() ) {
        std::string temp = "** ";
        temp += username;
        temp += " is diconnected";
        sendToAll(temp.c_str(), user_d, profiles, connections);
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
}

void clientHandler( char* buf, int user_d, int index,  profile* profiles, int* connections ) { //function for processing message from client and send correct message to some users
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
                sendToAll(temp_2.c_str(), index, profiles, connections);

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
            for (int i = 0; i < MAXCLIENT; i++) {
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
        else if ( (temp_1.substr(0, 8) == "\\private") &&  ( temp_1.c_str()[8] == ' ' ) ) { //sending private message
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
            if ( sendPrivateMsg(usrname, msg, index, profiles, connections) ) {
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
        else if( temp_1.substr(0, 5) == "\\quit"  &&  ( temp_1.c_str()[5] == ' ' || temp_1.c_str()[5] == 0 ) ){
            // quit from server
            if ( temp_1.size() > 5 ) {
                std::string message = temp_1.substr(6, temp_1.size() - 6);

                std::string temp_2 = "[";
                temp_2 += "farewell message from ";
                temp_2 += profiles[index].user_name;
                temp_2 += "]: ";
                temp_2 += message;

                sendToAll(temp_2.c_str(), index, profiles, connections);
                std::cout << temp_2 << std::endl;
            }

            disconnect(user_d, profiles[index].user_name, index, profiles, connections);
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
        sendToAll(str, index, profiles, connections);
        std::cout << str << std::endl;
    }

}