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

int consistingEnd( char* c_string ); //return 1 if c-string consists \n, \r, 0, etc; else return 0
void mapIntoStdSring( char* str ); //mapping consisisting of bufer to standar c-string
int correctName( char*  buf ); //check username for some rules
int nameIsNotUsed( std::string str, profile* profiles, int maxclient); //username is not used by other users
int findProfile( profile* profiles, int Count, std::string usrname); //return index in profiles array
std::string makePrivateMsg(std::string rcvr, std::string sender, std::string message); //creating correct form of private message
void processingStr(char* str); // mapping c-string to correct form (ignore excess spaces)
int spaceNum(const char* buf); // number of spaces in the c-string
int lengthOfFirtsWord(const char* buf); // size of the first word in the c-string
int firstDisc(int* connections, int max_client); // return the first unoccupied index
int initConnections(int* connections, int max_client); // initialization of the connections array

#endif //SERV_SERVER_H
