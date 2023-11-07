#include "network.h"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <unistd.h>

int custom_datagram_size = 8900;
int custom_gap = 71200;

network::network(int duration, network_target_location location, std::string bandwidth_limit)
    : m_duration(duration), m_target_location(location), m_bandwidth_limit(bandwidth_limit)
{
    char hostname[1024];
    gethostname(hostname, 1024);
    if (std::string(hostname) == "hpc1")
    {
        m_pc_type = hpc1;
    }
    else if (std::string(hostname) == "hpc2")
    {
        m_pc_type = hpc2;
    }
    else if (std::string(hostname) == "tpc1")
    {
        m_pc_type = tpc1;
    }
    else if (std::string(hostname) == "tpc2")
    {
        m_pc_type = tpc2;
    }
    else
    {
        throw std::runtime_error("[network][#2] PC is unknown. Could not get io device name.");
    }
}

network::~network()
{
    system("killall iperf3");
}

void network::server_start()
{
    std::cout << "[now] server_start" << std::endl;

    create_network_process(server, 19101);
    create_network_process(server, 19102);

    if ((m_target_location == hpc && m_pc_type != hpc2) || (m_target_location == tpc && m_pc_type != tpc2))
    {
        create_network_process(server, 19103);
    }
}

void network::client_start()
{
    switch (m_pc_type)
    {
    case hpc1:
        create_network_process(client, 19101, "10.0.0.101");
        create_network_process(client, 19101, "10.0.0.102");
        if (m_target_location == tpc)
            // create_network_process(client, 19103, "10.0.0.2");
            break;
    case hpc2:
        create_network_process(client, 19101, "10.0.0.1");
        create_network_process(client, 19102, "10.0.0.101");
        create_network_process(client, 19102, "10.0.0.102");
        break;
    case tpc1:
        create_network_process(client, 19102, "10.0.0.1");
        create_network_process(client, 19101, "10.0.0.2");
        if (m_target_location == hpc)
            create_network_process(client, 19103, "10.0.0.102");
        break;
    case tpc2:
        create_network_process(client, 19103, "10.0.0.1");
        create_network_process(client, 19102, "10.0.0.2");
        create_network_process(client, 19103, "10.0.0.101");
        break;
    }
}

bool network::create_network_process(process_type type, int port, std::string ip)
{
    pid_t fork_pid = fork();

    if (-1 == fork_pid)
    {
        return false;
    }
    else if (0 == fork_pid)
    {
        int ret = 0;

        switch (type)
        {
        case server:
        {
            ret = execlp("/usr/bin/iperf3", "/usr/bin/iperf3", "-s", "-p", std::to_string(port).c_str(), "-i", "60", nullptr);
            break;
        }
        case client:
        {
            ret = execlp("/usr/bin/iperf3", "/usr/bin/iperf3", "-u", "-p", std::to_string(port).c_str(), "-i", "60", "-c", ip.c_str(), "-b", m_bandwidth_limit.c_str(), "-t", std::to_string(m_duration).c_str(), nullptr);
            break;
        }
        }

        if (-1 == ret)
        {
            return false;
        }
    }

    return true;
}
