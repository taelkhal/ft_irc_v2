#include <iostream>
#include <fstream>
#include <vector>
#include <map>
// #include <thread>
// #include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstdlib>
#include <string>
#include <csignal>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <random>
#include "poll.h"

class client{
    public:
        int fd;
        client(int fd){this->fd = fd;};

};
class server{
    public:
        int sock;
        int listen_fd;
        std::vector<struct pollfd> _fd;

};

void addPollfd(int fd, server *ser) {
    struct pollfd newPollfd;
    newPollfd.fd = fd;
    newPollfd.events = POLLIN;
    newPollfd.revents = 0;
    ser->_fd.push_back(newPollfd);
}

void getSocket(server *ser)
{
    struct addrinfo hints;
    struct addrinfo *result;
    std::stringstream ssresult;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket (TCP)
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    ssresult << 9887;
    if ((getaddrinfo("localhost", ssresult.str().c_str(),
        &hints, &result)) != 0)
        throw ("error in addr");
    // std::cout << "getting the addr..."<< ser->socketAddr << "\n";
    if (result == NULL)
        throw ("error in result");
    if ((ser->sock = socket(result->ai_family,
                            result->ai_socktype, result->ai_protocol)) == -1){
        throw ("creating socket");
    }
    printf("socket for server is %d\n", ser->sock);
    int b = bind(ser->sock, result->ai_addr, result->ai_addrlen);
    if (b  == -1){

        close (ser->sock);
        throw std::runtime_error("error in bind");
    }
    // std::cout << "binding..." << b << "\n";
    freeaddrinfo(result);
    if ((ser->listen_fd = listen(ser->sock, FD_SETSIZE)) == -1){
        close(ser->sock);
        throw std::runtime_error("error in listen");
        // throw ("listen");
    }
    // else
    //     std::cout << "listining..." << ser[i].listen_fd <<"\n";
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(ser->sock, F_SETFL, flags | O_NONBLOCK);
    addPollfd(ser->sock, ser);
    printf("END CRATING SOCKETS...\n");
}

void mult(server *ser) {
    char buff[1024];
    std::vector <client> mycl;
    printf("start%lu\n",ser->_fd.size());
    while (1) {
        if (poll(ser->_fd.data(), ser->_fd.size(), -1) == -1)
            throw std::runtime_error("poll");

        for (size_t i = 0; i < ser->_fd.size(); i++) {
            if (ser->_fd[i].revents & POLLIN) {
                if (ser->sock == ser->_fd[i].fd) {
                    // New connection
                    printf("NEW CONNECTION\n");
                    int newFd = accept(ser->sock , NULL, NULL);
                    if (newFd == -1)
                        throw std::runtime_error("ERROR IN ACCEPTING\n");
                    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1)
                        throw std::runtime_error("Error: fcntl() failed");
                    client tmp(newFd);
                    addPollfd(newFd, ser);
                    mycl.push_back(tmp);
                    send(newFd, "connection accepted\n", 21, 0);
                    // printf("END OF CONNECTION\n");
                } else {
                    // Existing client
                    printf("CLIENT\n");
                    int nbytes = recv(ser->_fd[i].fd, buff, sizeof(buff), 0);
                    //
                    if (nbytes == -1)
                        throw std::runtime_error("error in recv\n");
                    else if (nbytes == 0) {
                        // Client disconnected
                        printf("Client disconnected\n");
                        close(ser->_fd[i].fd);
                        ser->_fd.erase(ser->_fd.begin() + i);
                    } else {
                        // Data received
                        buff[nbytes] = '\0'; // Null-terminate the received data
                        printf("Received data from client %d: %s\n", ser->_fd[i].fd, buff);
                    }
                }
            }
        }
    }
}

int main()
{
    try
    {
        server *ser;
        ser = new server;
        getSocket(ser);
        mult(ser);

    }
    catch(const std::exception& e)
    {
        std::cout << "errno set to " <<  strerror(errno) << '\n';
        std::cerr << e.what() << '\n';
    }
    
}
