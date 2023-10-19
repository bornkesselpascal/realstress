#include "communication.h"
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdexcept>

communication::udp::client::client(const std::string& address, int port, bool high_priority)
    : m_address(address)
    , m_port(port)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int r = getaddrinfo(m_address.c_str(), decimal_port, &hints, &m_addrinfo);
    if(r != 0 || m_addrinfo == 0) {
        throw std::runtime_error("[comm_udp_c] E01 - Invalid address (" + m_address + ") or socket (" + std::to_string(m_socket) + ").");
    }

    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1) {
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error("[comm_udp_c] E02 - Could not create socket for " + m_address + ":" + std::to_string(m_port) + ".");
    }

    if(high_priority) {
        int dscp_value = 63 << 2; // highest priority
        if(setsockopt(m_socket, IPPROTO_IP, IP_TOS, &dscp_value, sizeof(dscp_value)) < 0) {
            throw std::runtime_error("[comm_udp_c] E03 - Could not set DSCP for " + m_address + ":" + std::to_string(m_port) + ".");
        }
    }
}

communication::udp::client::~client() {
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

int communication::udp::client::send(const void *msg, size_t size) {
    return sendto(m_socket, msg, size, 0, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
}



communication::udp::server::server(const std::string& address, int port)
    : m_address(address)
    , m_port(port)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", m_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int r = getaddrinfo(m_address.c_str(), decimal_port, &hints, &m_addrinfo);
    if(r != 0 || m_addrinfo == NULL) {
        throw std::runtime_error("[comm_udp_s] E01 -  Invalid address (" + m_address + ") or socket (" + std::to_string(m_socket) + ").");
    }

    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        throw std::runtime_error("[comm_udp_s] E02 - Could not create socket for " + m_address + ":" + std::to_string(m_port) + ".");
    }

    int sock_size = 102428800;
    r = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &sock_size, sizeof(sock_size));
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        throw std::runtime_error("[comm_udp_s] E03 - Could not bind socket with " + m_address + ":" + std::to_string(m_socket) + ".");
    }

    int get_sock_size;
    socklen_t size = sizeof(get_sock_size);
    r = getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &get_sock_size, &size);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        throw std::runtime_error("[comm_udp_s] E04 - Could not bind socket with " + m_address + ":" + std::to_string(m_socket) + ".");
    }

    r = bind(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        throw std::runtime_error("[comm_udp_s] E05 - Could not bind socket with " + m_address + ":" + std::to_string(m_socket) + ".");
    }
}

communication::udp::server::~server() {
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}

/**
 * @brief communication::udp::server::receive
 *
 * @param msg      The buffer where the message will be saved.
 * @param max_size The size of the message buffer in bytes.
 * @return         Number of bytes read, -1 if an error occurs.
 */
int communication::udp::server::receive(void *msg, size_t max_size, bool timeout) {
    if(timeout) {
        struct pollfd fd;
        int res;

        fd.fd = m_socket;
        fd.events = POLLIN;
        res = ::poll(&fd, 1, 1000);

        if(0 == res || -1 == res) {
            return -1; // timeout
        }
        else if(-1 == res) {
            return -2; // error
        }
        else {
            return ::recv(m_socket, msg, max_size, 0);
        }
    }
    else {
        return ::recv(m_socket, msg, max_size, 0);
    }
}
