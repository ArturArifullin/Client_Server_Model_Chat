#ifndef SERV_PROFILE_H
#define SERV_PROFILE_H
#include <iostream>
#include <set>

class profile{
public:
    std::string user_name; // username of
    std::set<std::string> privates; // set of users, which was recieved message by this user
    std::string make_usr_name(char* buf); // method to mapping c-string to correct username
    int online = 0; // if user online value is 1, else- 0
};

#endif //SERV_PROFILE_H
