#ifndef SERV_SERVER_H
#define SERV_SERVER_H

#define MAXCLIENT 100
#define MINIMAL_SIZEOF_BUF 50
#define SIZEOF_MESSAGE 206
#define SIZEOF_BUF MINIMAL_SIZEOF_BUF+SIZEOF_MESSAGE
#define STDINPUT 0
#define MAX_SIZE_OF_USERNAME 12
#include <string>
#include "profile.h"

int consistingEnd( char* c_string );
void clientHandler( char* buf, int user_d, int id );
void mapIntoStdSring( char* str );
int correctName( char*  buf );
int nameIsNotUsed( std::string str, profile* profiles, int maxclient);
int findProfile( profile* profiles, int Count, std::string usrname);
std::string makePrivateMsg(std::string rcvr, std::string sender, std::string message);
void processingStr(char* str);
int spaceNum(const char* buf);

#endif //SERV_SERVER_H
