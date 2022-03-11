#ifndef SERV_PROFILE_H
#define SERV_PROFILE_H
#include <iostream>
#include <set>

class profile{
public:
    std::string user_name;
    std::set<std::string> privates;
    std::string make_usr_name(char* buf);
};

#endif //SERV_PROFILE_H
