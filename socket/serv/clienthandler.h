#ifndef SERV_CLIENTHANDLER_H
#define SERV_CLIENTHANDLER_H
#include "profile.h"

void sendToAll( const char* buf, int index, profile* profiles, int* connections);
int sendPrivateMsg( const std::string& usrname, std::string msg, int sender_id,  profile* profiles, int* connections);
void disconnect( int user_d, std::string& username, int index,  profile* profiles, int* connections );
void clientHandler( char* buf, int user_d, int index,  profile* profiles, int* connections );

#endif //SERV_CLIENTHANDLER_H
