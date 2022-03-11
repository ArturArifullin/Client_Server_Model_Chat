#include "profile.h"

std::string profile::make_usr_name(char* buf) {
    if (this->user_name.empty()){
        return 0;
    }

    std::string right_sq_br= "[";
    std::string temp_string = this->user_name;
    std::string left_sq_br= "]: ";
    std::string message(buf);
    return ( right_sq_br + temp_string + left_sq_br + message );
}

