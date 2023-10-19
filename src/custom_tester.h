#ifndef CUSTOM_TESTER_H
#define CUSTOM_TESTER_H

#include "communication.h"
#include "test_results.h"
#include <string>
#include <unistd.h>

struct custom_tester_client_description {
    std::string client_ip;
    int port = 8090;
    int gap;
    struct datagram {
        int  size;
    } datagram;
    int duration;
};

struct custom_tester_server_description {
    std::string server_ip;
    int port = 8090;
    struct datagram {
        int  size;
    } datagram;
};

class custom_tester_client
{
public:
    custom_tester_client(custom_tester_client_description description);
    void run();

private:
    custom_tester_client_description m_description;
    communication::udp::client m_comm_client;
};

class custom_tester_server
{
public:
    custom_tester_server(custom_tester_server_description description);
    void run();

private:
    custom_tester_server_description m_description;
    communication::udp::server m_comm_server;
};

#endif // CUSTOM_TESTER_H
