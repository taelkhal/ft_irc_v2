#include "Server.hpp"
#include "irc.hpp"

bool Server::_signal = false;

Server::Server() {}

Server::~Server() {}


void Server::set_av(std::string av)
{
    this->av = av;
}

std::string Server::get_av()
{
    return this->av;
}

void Server::set_admin(std::string admin)
{
    this->admin = admin;
}

std::string Server::get_admin()
{
    return this->admin;
}

void Server::parseArgs(int ac, char **av) {
    if (ac != 3)
        throw std::runtime_error("Usage: ./ircserv <port> <password>");
    std::string port(av[1]);
    std::string pwd(av[2]);
    if (port.empty() || port.find_first_not_of("0123456789") != std::string::npos)
        throw std::runtime_error("Error: Invalid arguments");

    long _port = atol(av[1]);
    if (!(_port >= 1 && _port <= 65535))
        throw std::runtime_error("Error: Invalid arguments");

    if (pwd.empty())
        throw std::runtime_error("Error: Invalid arguments");

    this->_port = _port;
    this->_password = pwd;
}

void Server::receiveSignal(int signum) {
    _signal = true;
    (void)signum;
}

void Server::init() {
    signal(SIGINT, receiveSignal);
    signal(SIGQUIT, receiveSignal);

    createServerSocket();
    std::cout << ">>> SERVER STARTED <<<" << std::endl;
    std::cout << "Waiting for connections..." << std::endl;
}

void Server::run() {
    while (!_signal) {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret == -1)
            throw std::runtime_error("Error: poll() failed");

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocketFd)
                    handleClientConnection();
                else
                    handleClientData(_fds[i].fd);
            }
        }
    }
    closeFds();
}

void Server::createServerSocket() {
    _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("Error: failed to create socket");

    int optval = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        throw std::runtime_error("Error: setsockopt() failed");

    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Error: fcntl() failed");

    bindServerSocket();

    if (listen(_serverSocketFd, SOMAXCONN) == -1)
        throw std::runtime_error("Error: listen() failed");

    addPollfd(_serverSocketFd, POLLIN, 0);
}

void Server::bindServerSocket() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(_port);
    sa.sin_addr.s_addr = INADDR_ANY;
    if (bind(_serverSocketFd, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        throw std::runtime_error("Error: failed to bind socket");
    }
}

void Server::addPollfd(int fd, short events, short revents) {
    struct pollfd newPollfd;
    newPollfd.fd = fd;
    newPollfd.events = events;
    newPollfd.revents = revents;
    _fds.push_back(newPollfd);
}

void Server::handleClientConnection() {
    struct sockaddr_in client_addr;
    socklen_t clientAddrSize = sizeof(sockaddr_in);
    int newFd = accept(_serverSocketFd, (struct sockaddr *)&client_addr, &clientAddrSize);
    if (newFd == -1) {
        throw std::runtime_error("Error: accept() failed");
    }

    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1) // set non-blocking to the new socket fd it's mean that the socket will not block the program
        throw std::runtime_error("Error: fcntl() failed");
    
    std::string entrepass = "Enter Password Please : \n";

    send(newFd, entrepass.c_str(), entrepass.size(), 0); 

    addPollfd(newFd, POLLIN, 0);
    _clients.push_back(Client(newFd, inet_ntoa((client_addr.sin_addr))));
    std::cout << "Client <" << newFd << "> Connected" << std::endl;
}

void Server::handleClientData(int fd) {
    std::string command;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead;
    while ((bytesRead = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        bool foundEof = false;
        for (ssize_t i = 0; i < bytesRead; ++i) {
            if (buffer[i] == '\n') {
                foundEof = true;
                break;
            }
        }

        if (!foundEof) {
            buffer[bytesRead] = '\0';
            command += buffer;
        } else {
            command.append(buffer, bytesRead - 1);
            std::cout << "Received data from client " << fd << ": " << command << std::endl;
            // printf("----->{%s}\n", command.substr(0, 7).c_str());
            // process the command here
            
            // std::string user_c;
            this->fd_u = fd;
            bool is_admin = false;
            // std::string savenick;
            // std::cout << "fd_u" << fd_u << std::endl;
            if (command.substr(0, 5) == "pass " || command.substr(0, 5) == "PASS ")
            {
                std::string s_av = get_av();
                size_t space_pos = command.find(' ');

                if (space_pos != std::string::npos)
                {
                    std::string new_c = command.substr(space_pos + 1);
                    new_c = new_c.substr(0, new_c.size() - 1);
                    if (new_c != s_av) 
                    {
                        std::string msg_error = "Wrong Password\n";
                        send(fd, msg_error.c_str(), msg_error.size(), 0);
                    } 
                    else 
                    {
                        std::string msg = "Enter Your Nickname :\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                }
            }
            else if (command.substr(0, 5) == "nick " || command.substr(0, 5) == "NICK ")
            {
                std::string nickname;
                

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos)
                {
                    // static int t =  0;
                    nickname = command.substr(space_pos + 1);
                    nickname = nickname.substr(0, nickname.size() - 1);
                    std::cout << "nickmok-->" << nickname << std::endl;
                    users_fd[nickname].push_back(fd);

                    std::string msg = "Please Enter Your Username : \n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
            else if(command.substr(0, 5) == "user " || command.substr(0, 5) == "USER ")
            {
                std::string username;

                size_t space_pos = command.find(' ');

                if (space_pos != std::string::npos)
                {
                    // std::string msg_error = "Enter your username:\n";
                    // send(fd, msg_error.c_str(), msg_error.size(), 0);
                    username = command.substr(space_pos + 1);
                    username = username.substr(0, username.size() - 1);


                    std::map<std::string, std::vector<int> >::iterator it;
                    std::vector<int>::iterator it1;
                    std::string user_c;
                    for (it = users_fd.begin(); it != users_fd.end(); ++it)
                    {
                        std::vector<int>::iterator vec_it;
                        for(vec_it = it->second.begin(); vec_it != it->second.end(); ++vec_it)
                        {
                            if (*vec_it == fd)
                            {
                                user_c = it->first;
                            }
                        }
                    }
                    welcome(user_c, fd);
                }
            }
            else if (command.substr(0, 5) == "join " || command.substr(0, 5) == "JOIN ")
            {
                std::string channelname;

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos) {
                    channelname = command.substr(space_pos + 1);
                    channelname = channelname.substr(0, channelname.size() - 1);

                    std::string user_c;
                    // bool is_admin = false;

                    // Find the username associated with the file descriptor
                    for (std::map<std::string, std::vector<int> >::iterator it = users_fd.begin(); it != users_fd.end(); ++it) 
                    {
                        std::vector<int>::iterator vec_it = std::find(it->second.begin(), it->second.end(), fd);
                        if (vec_it != it->second.end()) {
                            user_c = it->first;
                            break;
                        }
                    }

                    create_channel(channelname, user_c, fd);

                    for (std::map<std::string, std::vector<int> >::iterator it = users_fd.begin(); it != users_fd.end(); ++it) 
                    {
                        std::vector<int>::iterator vec_it = std::find(it->second.begin(), it->second.end(), fd);
                        if (vec_it != it->second.end()) 
                        {
                            user_c = it->first;
                            // Check if the user is an admin
                            std::map<std::string, std::vector<std::string> >::iterator it_1 = channel.find(channelname);
                            if (it_1 != channel.end()) 
                            {
                                // Iterate over all members of the channel
                                for (size_t i = 0; i < it_1->second.size(); ++i) 
                                {
                                    // Check if the user is the admin of the channel
                                    std::cout << "name:" << it_1->second[i] << std::endl;
                                    if (it_1->second[i][0] == '@' && it_1->second[i].substr(1) == user_c) 
                                    {
                                        std::cout << user_c << " is an admin" << std::endl;
                                        is_admin = true;
                                        this->admin = user_c;
                                        set_admin(user_c);
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    std::string joinMessage = ":" + user_c + " JOIN " + channelname + "\r\n";
                    std::string modeMessage = ":irc.ChatWladMina MODE " + channelname + " +nt\r\n";

                    std::map<std::string, std::vector<std::string> >::iterator it_1 = channel.find(channelname);
                    std::string namesMessage = ":irc.ChatWladMinah 353 " + user_c +  " = " + channelname + " :";
                    std::vector<std::string>::iterator it_v;
                    for (it_v = it_1->second.begin(); it_v != it_1->second.end(); it_v++)
                    {
                        if (is_admin == true) 
                            namesMessage += "@" + *it_v;
                        else 
                            namesMessage += *it_v;
                        if (it_v < it_1->second.end() - 1)
                            namesMessage += " ";
                    }
                    namesMessage += "\n";

                    std::string endOfNamesMessage = ":irc.ChatWladMina 366 " + user_c + " " + channelname + " :End of /NAMES list.\r\n";
                    std::string channelMessage = ":irc.ChatWladMina 354 " + channelname + "\r\n";

                    // Send messages to the client
                    send(fd, joinMessage.c_str(), joinMessage.length(), 0);
                    send(fd, modeMessage.c_str(), modeMessage.length(), 0);
                    send(fd, namesMessage.c_str(), namesMessage.length(), 0);
                    send(fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
                    send(fd, channelMessage.c_str(), channelMessage.length(), 0);
                }
            }
            else if (command.substr(0, 7) == "privmsg" || command.substr(0, 7) == "PRIVMSG")
            {
                std::string p_msg;

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos)
                {
                    p_msg = command.substr(space_pos + 1);
                    p_msg = p_msg.substr(0, p_msg.size() - 1);
                    processMessage(command, fd);
                }
            }
            else if (command.substr(0, 5) == "kick " || command.substr(0, 5) == "KICK ")
            {
                std::string p_msg;

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos)
                {
                    p_msg = command.substr(space_pos + 1);
                    p_msg = p_msg.substr(0, p_msg.size() - 1);
                }
                std::istringstream iss(command);
                std::string command, channel_kicked_from, user_kicked, reason;
                // Parse the message
                iss >> command;
                // Check the command type
                // std::cout << "command :"  <<command << "." << std::endl;
                // if (command == "privmsg" || command == "PRIVMSG") {
                    // Extract the target channel and message text
                iss >> channel_kicked_from;
                iss >> user_kicked;
                std::getline(iss, reason); // Read the rest of the line as message text
                reason = reason.substr(1);
                reason = reason.substr(0, reason.size());
                std::cout << "command kick : " << command << std::endl;
                std::cout << "bool : " << is_admin << '.' << std::endl;
                // if (is_admin == true)
                std::cout << "admin :"  << get_admin() << std::endl;
                if(user_kicked != get_admin())
                {
                    kick_memeber(channel_kicked_from, user_kicked, reason, fd);
                }
                else
                {
                    std::string not_admin = ":" + user_kicked + " PRIVMSG " + channel_kicked_from + " You are not authorized to execute this command kick admin " + user_kicked + "\r\n";
                    send(fd, not_admin.c_str(), not_admin.size(), 0);
                }

            }
            else if(command.substr(0, 7) == "invite " || command.substr(0, 7) == "INVITE ")
            {
                std::string invite;

                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos)
                {
                    invite = command.substr(space_pos + 1);
                    invite = invite.substr(0, invite.size() - 1);

                    std::istringstream iss(command);
                    std::string command, channel_invited_from, user_invited;

                    iss >> command;
                    iss >> user_invited;
                    iss >> channel_invited_from;
                    std::cout << "user_invited :" << user_invited << "."  << std::endl;
                    std::cout << "channel invited  from :" << channel_invited_from << "."  << std::endl;

                    std::string invite_msg = ":" + get_admin() + " INVITE " + user_invited + " :" + channel_invited_from + "\r\n";
                    int invited_fd = get_fd_users(user_invited);
                    send(fd, invite_msg.c_str(), invite_msg.size(), 0);
                    send(invited_fd, invite_msg.c_str(), invite_msg.size(), 0);
                }

            }
            break;
        }
    }

    if (bytesRead == 0) {
        std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        clientCleanup(fd);
    } else if (bytesRead == -1) {
        std::cerr << "Error reading data from client <" << fd << ">" << std::endl;
        clientCleanup(fd);
    }
}

void Server::clientCleanup(int fd) {
    for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (it->fd == fd) {
            _fds.erase(it);
            close(fd);
            break;
        }
    }

    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->getFd() == fd) {
            _clients.erase(it);
            break;
        }
    }
}

// std::string Server::formatCreationTime() {
//     std::time_put currentTime = std::time(NULL);
//     std::tm* localtime = std::locale(&currentTime);

//     char buffer[80]; 
//     std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localtime);
//     return std::string(buffer);
// }

// std::string Server::constructCreationTimeMessage(const std::string& channelName) {
//     std::stringstream ss;
//     ss << "Channel #" << channelName << " created " << formatCreationTime();
//     return ss.str();
// }
void Server::welcome(const std::string& nickname, int fd)
{
    std::string one = ":irc.ChatWladMina 001 " + nickname + " :Welcome to the ChatWladMina Network, " + nickname + '\n';
    std::string two = ":irc.ChatWladMina 002 " + nickname + " :Your host is ChatWladMina, running version 4.5" + '\n';
    std::string tre = ":irc.ChatWladMina 003 " + nickname + " :This server was created " + '\n';
    std::string foor = ":irc.ChatWladMina 004 " + nickname + " ChatWladMina ChatWladMina(enterprise)-2.3(12)-netty(5.4c)-proxy(0.9) ChatWladMina" + '\n';
    send(fd, one.c_str(), one.length(), 0);
    send(fd, two.c_str(), two.length(), 0);
    send(fd, tre.c_str(), tre.length(), 0);
    send(fd, foor.c_str(), foor.length(), 0);
}

void Server::closeFds() {
    for (size_t i = 0; i < _clients.size(); i++){
        int fd = _clients[i].getFd();
        std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        close(fd);
    }

    if (_serverSocketFd != -1)
        close(_serverSocketFd);

    _fds.clear();
}
void Server::ping(const std::string& command, int fd) {
    std::istringstream iss(command);
    std::string serverHostname = command.substr(5);
    std::string pongMessage = "PONG " + serverHostname + "\r\n";
    send(fd, pongMessage.c_str(), pongMessage.length(), 0);
    std::cout << "PONG" << std::endl;
}

void Server::processMessage(const std::string& message, int fd)
{
    std::istringstream iss(message);
    std::string command, target, text;
    // Parse the message
    iss >> command;
    // Check the command type
    // std::cout << "command :"  <<command << "." << std::endl;
    // if (command == "privmsg" || command == "PRIVMSG") {
        // Extract the target channel and message text
    iss >> target;
    std::getline(iss, text); // Read the rest of the line as message text
    text = text.substr(1);   // Remove leading space
    // Now you have the target channel and message text
    

    //find nick name
    std::map<std::string, std::vector<int> >::iterator it;;
    // std::vector<int>::iterator itt = it->second.begin();
    // for (; itt != it->second.end(); itt++)
    //     std::cout << "loop=" << *itt << std::endl;
    // std::vector<int>::iterator it1;
    // std::string user_c;
    // for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    // {
    //     std::cout << "loop:" << *it1 << " |the fd is: " << fd << std::endl;
    //     if (*it1 == fd)
    //     {
    //         user_c = it->first;
    //         fd_u = fd;
    //     }
    // }

    std::vector<int>::iterator it1;
    std::string user_c;
    for (it = users_fd.begin(); it != users_fd.end(); ++it)
    {
        std::vector<int>::iterator vec_it;
        for(vec_it = it->second.begin(); vec_it != it->second.end(); ++vec_it)
        {
            // std::cout << "loop:" << *it1 << " |the fd is: " << fd << std::endl;
            if (*vec_it == fd)
            {
                user_c = it->first;
                fd_u = fd;
            }
        }
    }
    text = text.substr(0, text.size() - 1);
    std::string tmp_mess = ":" + user_c + " PRIVMSG " + target + " " + text + "\r\n";
    sendMessageToChannel(target, tmp_mess);
}

void Server::sendMessageToChannel(const std::string& channell, const std::string& message) 
{

    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channell);
    std::cout << "here" << std::endl;
    if(it == channel.end())
    {
        // for(it = channel.begin(); it != channel.end();it++)
        // {
        //     std::cout << "first : "<<it->first << "."  << std::endl;
        // }
        std::cout << "Channel '" << channell << "' does not exist." << std::endl;
    }
    else
    {
        std::vector<int> fds = send_msg(channell);
        std::vector<int>::iterator in;

        for (size_t i = 0; i < fds.size(); ++i) 
        {
            int user = fds[i];
            if (fd_u != fds[i])
                sendMessageToUser(user, message);
        }

    } 
}

void Server::sendMessageToUser(int user, const std::string& message) 
{
    // std::cout << "message22222--->" << message << std::endl;
    send(user, message.c_str(), message.size(), 0);
    std::cout << "Sending message to user '" << user << "': " << message ;
}
