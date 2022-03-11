#include "server.h"
#include "profile.h"
#include <cstring>
#include <queue>


int consistingEnd( char* c_string ){
    if ( c_string[0] == 0 )
        return 0;

    for ( int i = 0; i < SIZEOF_BUF; i++ ){
        if ( !c_string[i] )
            return 1;
        if ( c_string[i] == '\n' || c_string[i] == '\r' ){
            return 1;
        }
        if ( (i < SIZEOF_BUF - 1) || ( c_string[i] == '\n' && c_string[i + 1] == '\r')
             || ( c_string[i] == '\r' && c_string[i + 1] == '\n') ){
            return 1;
        }
    }

    return 0;
}

int consistingSpace( char* buf ){
    int i;
    for( i = 0; ( buf[i] != '\0') && ( buf[i] != '\n') && ( buf[i] != '\r'); i++ ){
        if (buf[i] == ' ')
            return 1;
    }
    return 0;
}

int correctName( char*  buf ){
    if ( consistingSpace(buf) )
        return 0;
    if (  ( buf[0] == '\0') || ( buf[0] == '\n') || ( buf[0] == '\r') )
        return 0;
    if ( strlen(buf) > MAX_SIZE_OF_USERNAME )
        return 0;
    if ( buf[0] == '\\' || buf[0] == '/' || buf[0] == '*' || buf[0] == '#' )
        return 0;
    return 1;
}

void mapIntoStdSring( char* str ){
    for ( int i = 0; i < SIZEOF_BUF; i++ ){
        if ( str[i] == '\n' || str[i] == '\r' ) {
            str[i] = 0;
            break;
        }
    }
}

int nameIsNotUsed( std::string str, profile* profiles, int maxclient ){
    for ( int i = 0; (i < maxclient) || (profiles[i].user_name.empty()); i++) {
        if (str == profiles[i].user_name)
            return 0;
    }
    return 1;
}

int findProfile( profile* profiles, int Count, std::string usrname){
    for ( int i = 0; i < Count; i++ ){
        if ( profiles[i].user_name == usrname){
            return i;
        }
    }
    return -1;
}

std::string makePrivateMsg(std::string rcvr, std::string sender, std::string message){
    std::string t= "*[Private from ";
    t += sender;
    t += " to ";
    t += rcvr;
    t+= "]: ";
    t+= message;
    return t;
}

void processingStr(char* str){
    std::queue <char> queue_char;
    int begin = 1;
    int space_presence = 0;

    for( int i = 0; str[i] != 0; i++ ){
        if ( begin == 1 && str[i] != ' '){
            begin = 0;
            queue_char.push(str[i]);
        }
        else if ( space_presence == 0 && str[i] == ' ' && begin == 0 ){
            space_presence = 1;
            queue_char.push(str[i]);
        }
        else if ( space_presence == 1 && str[i] != ' ' && begin == 0){
            space_presence = 0;
            queue_char.push(str[i]);
        }
        else if ( space_presence == 0 && str[i] != ' '){
            queue_char.push(str[i]);
        }
    }

    int size = queue_char.size();
    for( int i = 0; i < size; i++ ){
        str[i] = queue_char.front();
        queue_char.pop();
    }
    str[size] = 0;
}

int spaceNum(const char* buf){
    int count_space= 0;
    for ( int i = 0; buf[i] != 0 ; i++ ){
        if ( buf[i] == ' ' )
            count_space++;
    }
    return count_space;
}