#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace communication {
namespace udp {

enum message_type {
    CDATA_MSG,
    CSTOP_MSG,
};

class client {
public:
    client(const std::string& address, int port, bool high_priority = false);
    ~client();

    int         get_socket()  const { return m_socket;  };
    int         get_port()    const { return m_port;    };
    std::string get_address() const { return m_address; };

    int send(const void *msg, size_t size);

private:
    int m_socket;
    int m_port;
    std::string m_address;
    struct addrinfo *m_addrinfo;
};

class server
{
public:
    server(const std::string& address, int port);
    ~server();

    int         get_socket()  const { return m_socket;  };
    int         get_port()    const { return m_port;    };
    std::string get_address() const { return m_address; };

    int receive(void *msg, size_t max_size, bool timeout = false);

private:
    int m_socket;
    int m_port;
    std::string m_address;
    struct addrinfo *m_addrinfo;
};

} // namespace udp
namespace tcp {



} // namespace tcp
} // namespace communication

#endif // COMMUNICATION_H
