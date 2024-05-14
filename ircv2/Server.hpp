#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <sstream>
#define BUFFER_SIZE 1024

#define JOIN_MESSAGE(nickname, channelName) (":" + nickname + " JOIN #" + channelName + "\n")
#define MODE_MESSAGE(channelName) (":irc.l9oroch MODE #" + channelName + " +nt\n")
#define NAMES_MESSAGE(nickname, channelName) (":irc.l9oroch 353 " + nickname + " = #" + channelName + " :@" + nickname + "\n")
#define NAMES_MESSAGE2(nickname, channelName) (":irc.l9oroch 353 " + nickname + " @ #" + channelName + " :")
#define END_OF_NAMES_MESSAGE(nickname, channelName) (":irc.l9oroch 366 " + nickname + " #" + channelName + " :End of /NAMES list.\n")
#define CHANNEL_MESSAGE(channelName) (":irc.l9oroch 354 " + channelName +"\n")
#define TOPIC_MESSAGE(nickname, channelName, topic) (":irc.l9oroch 332 " + nickname + " #" + channelName + " :" + topic + " https://irc.com\n")
#define TOPIC_MESSAGE2(nicknamesender, channelname, topic) (":" + nicknamesender + " TOPIC #" + channelname + " :" + topic + "\n")

class Server {
    private:
        int _port;
        int _serverSocketFd;
        std::string av;
        int fd_u;
        std::string admin;
        std::string user_n;
        
        std::string nick_n;
        static bool _signal;
        std::string _password;
        std::map<std::string, std::vector<std::string> > channel;
        std::map<std::string, std::vector<int> > users_fd;
        std::vector<std::string> nicknames;
        std::string channel_n;
        std::vector<struct pollfd> _fds;
        std::vector<Client> _clients;
    public:
        Server();
        ~Server();
        std::string user_n1;

        void parseArgs(int ac, char **av);
        static void receiveSignal(int signum);
        void init();
        void run();

        void createServerSocket();
        void bindServerSocket();
        void addPollfd(int fd, short events, short revents);
        void handleClientConnection();
        void handleClientData(int fd);
        void clientCleanup(int fd);
        void closeFds();
        void set_av(std::string av);
        std::string get_av();
        void welcome(const std::string& nickname, int fd);
        void    set_fd_users(const std::string &user_name,int fd);
        bool    if_member(std::string user_inchannel, std::string channels_name);
        std::vector<int>    send_msg(const std::string &channel_msg);
        int     get_fd_users(std::string &users_in_channel);
        void    create_channel(const std::string &channel_name,const std::string& user_name, int fd);
        void processMessage(const std::string& messag, int fd);
        void sendMessageToChannel(const std::string& channel, const std::string& message);
        void sendMessageToUser(int user, const std::string& message);
        std::string constructCreationTimeMessage(const std::string& channelName);
        std::string formatCreationTime();
        void ping(const std::string& command, int fd);
        void kick_memeber(std::string &channel_kicked_from, std::string &user_kicked, std::string reason, int fd);
        
};

#endif