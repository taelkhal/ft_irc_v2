#include "Server.hpp"

// void Server::create_channel(const std::string &channel_name,const std::string& user_name, int fd)
// {
//     std::map<std::string, std::vector<std::string> >::iterator it;
//     if(channel.find(channel_name) == channel.end())
//     { 
//         channel[channel_name].push_back(user_name);
//     }
//     else
//     {
//         channel[channel_name].push_back(user_name);
//         std::cout << "channel already exist" << std::endl;
//         // return false;
//     }
// }

void Server::create_channel(const std::string &channel_name, const std::string& user_name, int fd)
{
    std::map<std::string, std::vector<std::string> >::iterator it = channel.find(channel_name);
    if (it == channel.end())
    { 
        std::vector<std::string> users;
        users.push_back("@" + user_name); // Admin user
        channel[channel_name] = users;
    }
    else
    {
        channel[channel_name].push_back(user_name);
        std::cout << "channel already exists" << std::endl;
    }
}
            

void Server::set_fd_users(const std::string &user_name,int fd)
{
    if(users_fd.find(user_name) == users_fd.end())
    {
        users_fd[user_name].push_back(fd);
    }
    else
    {
        std::cout << "user already exist" << std::endl;
    }
}
bool Server::if_member(std::string user_inchannel, std::string channels_name)
{
    std::vector<std::string>::iterator it1;
    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channels_name);
    if(it != channel.end()){
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            // std::cout << "fd: " << fd << std::endl;
            // std::cout << "it1 : " << *it1 << std::endl;
            if(*it1 == user_inchannel)
                return true;
        }
    }
    return(false);
}
int Server::get_fd_users(std::string &users_in_channel)
{
    // std::cout << users_in_channel << std::endl;

    // exit (0);
    int fd;
    std::vector<int>::iterator it1;
    std::map<std::string, std::vector<int> >::iterator it;
    if (users_in_channel[0] == '@')
    {
        it = users_fd.find(&users_in_channel[1]);
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            fd = *it1;
        }
    }
    else
    {
        it = users_fd.find(users_in_channel);
        for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            fd = *it1;
        }
    }
    // it = users_fd.find(&users_in_channel[1]);
    // std::cout << "------>>>>>>" << users_in_channel << std::endl;
    // for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    // {
    //     fd = *it1;
    // }
    return fd;
}

// void Server::processMessage(const std::string& message) 
// {
//         std::istringstream iss(message);
//         std::string command, target, text;

//         // Parse the message
//         iss >> command;

//         // Check the command type
//         if (command == "/msg") {
//             // Extract the target channel and message text
//             iss >> target;
//             std::getline(iss, text); // Read the rest of the line as message text
//             text = text.substr(1);   // Remove leading space

//             // Now you have the target channel and message text
//             std::cout << "Target channel: " << target << std::endl;
//             std::cout << "Message text: " << text << std::endl;

//             // Process the message further (send to target channel, etc.)
//             sendMessage(target, text);
//         } else {
//             std::cout << "Invalid command: " << command << std::endl;
//         }
//     }

// void sendMessage(const std::string& channel, const std::string& message) 
// {
//         // Your implementation to send the message to the specified channel
//         std::cout << "Sending message to channel '" << channel << "': " << message << std::endl;
// }

std::vector<int> Server::send_msg(const std::string &channel_msgs)
{
    std::vector<std::string>::iterator it1;
    std::map<std::string, std::vector<std::string> >::iterator it;
    it = channel.find(channel_msgs);
    std::vector<int> fds;
    for(it1 = it->second.begin(); it1 != it->second.end(); it1++)
    {
        int fd = get_fd_users(*it1);

        fds.push_back(fd);
    }
    return (fds);
}
// int main() {
//     channels server;
//     std::map<std::string, std::vector<int> >::iterator it;
//     std::vector<int>::iterator it1;

//     // Create a channel
//     server.creat_channel("#general", "aymen");

//     // Try to create the same channel again
//     server.creat_channel("#gene", "taha");
//     server.creat_channel("#ger", "user");
//     // server.creat_channel("#general", "asee");
//     // server.creat_channel("#gene");
//     // for(it = server.channel.begin(); it != server.channel.end(); it++) {
//     //     std::cout << "Channel: " << it->first << std::endl;
//     //     std::cout << "Users: ";
//     //     for(it1 = it->second.begin(); it1 != it->second.end(); it1++) {
//     //         std::cout << *it1 << " ";
//     //     }
//     //     std::cout << std::endl;
//     // }

//     server.get_fd_users("aymen", 1);
//     server.get_fd_users("taha", 2);
//     server.get_fd_users("ss", 3); // Try adding existing user
//     server.get_fd_users("user3", 4);
//     for(it = server.users_fd.begin(); it != server.users_fd.end(); it++) {
//         std::cout << "user: " << it->first << std::endl;
//         std::cout << "fd: ";
//         for(it1 = it->second.begin(); it1 != it->second.end(); it1++) {
//             std::cout << *it1 << " ";
//         }
//         std::cout << std::endl;
//     }
//     return 0;
// }
// int main() {
//     channels manager;

//     // Test create_channel function
//     std::cout << "Testing create_channel function:" << std::endl;
//     bool result1 = manager.creat_channel("#channel1", "user1");
//     bool result2 = manager.creat_channel("#channel2", "user2");
//     bool result3 = manager.creat_channel("#channel1", "user3"); // Try creating existing channel
//     std::cout << "Result 1: " << (result1 ? "Success" : "Failed") << std::endl;
//     std::cout << "Result 2: " << (result2 ? "Success" : "Failed") << std::endl;
//     std::cout << "Result 3: " << (result3 ? "Success" : "Failed") << std::endl;

//     // Test get_fd_users function
//     std::cout << "\nTesting get_fd_users function:" << std::endl;
//     manager.set_fd_users("user1", 1);
//     manager.set_fd_users("user2", 2);
//     manager.set_fd_users("user1", 3); // Try adding existing user
//     manager.set_fd_users("user3", 4);

//     // Test if_member function
//     std::cout << "\nTesting if_member function:" << std::endl;
//     bool result4 = manager.if_member("user1", "#channel1"); // Check if user with fd 1 is a member of #channel1
//     bool result5 = manager.if_member("user2", "#channel1"); // Check if user with fd 2 is a member of #channel1
//     bool result6 = manager.if_member("user1", "#channel2"); // Check if user with fd 3 is a member of #channel2
//     std::cout << "Result 4: " << (result4 ? "User is a member" : "User is not a member") << std::endl;
//     std::cout << "Result 5: " << (result5 ? "User is a member" : "User is not a member") << std::endl;
//     std::cout << "Result 6: " << (result6 ? "User is a member" : "User is not a member") << std::endl;
//     manager.send_msg("#channel1");
//     manager.send_msg("#channel2");

//     return 0;
// }
//msg from the user check the channel where the msg sent and check the users and fds to send msg to everyone 


//the last one joined send all the messages
//admn problem