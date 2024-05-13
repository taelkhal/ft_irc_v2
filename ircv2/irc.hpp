#ifndef IRC_HPP
#define IRC_HPP

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include <cstring>
#include <unistd.h>
#include <map>

class channels 
{
    private:
        std::map<std::string, std::vector<std::string> > channel;
        std::map<std::string, std::vector<int> > users_fd;
    public:
        channels();
        ~channels();
        void    set_fd_users(const std::string &user_name,int fd);
        bool    if_member(std::string user_inchannel, std::string channels_name);
        void    send_msg(const std::string &channel_msg);
        int     get_fd_users(std::string &users_in_channel);
        void    creat_channel(const std::string &channel_name,const std::string& user_name);
};
#endif