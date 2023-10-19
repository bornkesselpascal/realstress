#include "network.h"
#include "custom_tester.h"
#include <stdexcept>
#include <unistd.h>

int custom_datagram_size = 8900;
int custom_gap = 71200;

bool create_iperf_process(process_type type, int port, std::string ip);
bool create_custom_process(process_type type, int port, std::string ip);

network::network(int duration, target_location location, network_method method, std::string bandwidth_limit = "1G")
    : m_duration(duration), m_target_location(location), m_method(method), m_bandwidth_limit(bandwidth_limit)
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
    switch (m_method)
    {
    case iperf:
        system("killall iperf3");
        break;
    case custom:
        for (auto &thread : m_custom_threads)
        {
            thread->join();
        }
        break;
    }
}

void network::server_start()
{
    switch (m_method)
    {
    case iperf:
        create_iperf_process(server, 19101);
        create_iperf_process(server, 19102);

        if ((m_target_location == hpc && m_pc_type != hpc2) || (m_target_location == tpc && m_pc_type != tpc2))
        {
            create_iperf_process(server, 19103);
        }
        break;
    case custom:
        switch (m_pc_type)
        {
        case hpc1:
            create_iperf_process(server, 19101, "10.0.0.1");
            create_iperf_process(server, 19102, "10.0.0.1");
            create_iperf_process(server, 19103, "10.0.0.1");
            break;
        case hpc2:
            create_iperf_process(server, 19101, "10.0.0.2");
            create_iperf_process(server, 19102, "10.0.0.2");
            if (m_target_location == tpc)
                create_iperf_process(server, 19103, "10.0.0.2");
            break;
        case tpc1:
            create_iperf_process(server, 19101, "10.0.0.101");
            create_iperf_process(server, 19102, "10.0.0.101");
            create_iperf_process(server, 19103, "10.0.0.101");
            break;
        case tpc2:
            create_iperf_process(server, 19101, "10.0.0.102");
            create_iperf_process(server, 19102, "10.0.0.102");
            if (m_target_location == hpc)
                create_iperf_process(server, 19103, "10.0.0.102");
            break;
        }
        break;
    }
}

void network::client_start()
{
    switch (m_pc_type)
    {
    case hpc1:
        create_iperf_process(client, 19101, "10.0.0.101");
        create_iperf_process(client, 19101, "10.0.0.102");
        if (m_target_location == tpc)
            create_iperf_process(client, 19103, "10.0.0.2");
        break;
    case hpc2:
        create_iperf_process(client, 19101, "10.0.0.1");
        create_iperf_process(client, 19102, "10.0.0.101");
        create_iperf_process(client, 19102, "10.0.0.102");
        break;
    case tpc1:
        create_iperf_process(client, 19102, "10.0.0.1");
        create_iperf_process(client, 19101, "10.0.0.2");
        if (m_target_location == hpc)
            create_iperf_process(client, 19103, "10.0.0.102");
        break;
    case tpc2:
        create_iperf_process(client, 19103, "10.0.0.1");
        create_iperf_process(client, 19102, "10.0.0.2");
        create_iperf_process(client, 19103, "10.0.0.101");
        break;
    }
}

bool network::create_network_process(process_type type, int port, std::string ip = "")
{
    switch (m_method)
    {
    case iperf:
        return create_iperf_process(type, port, ip);
    case custom:
        return create_custom_process(type, port, ip);
    }
}

bool create_iperf_process(process_type type, int port, std::string ip)
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

bool create_custom_process(process_type type, int port, std::string ip)
{
    switch (type)
    {
    case (client):
    {
        custom_tester_client_description client_desc;
        client_desc.client_ip = ip;
        client_desc.port = port;
        client_desc.gap = custom_gap;
        client_desc.datagram.size = custom_datagram_size;
        client_desc.duration = m_duration;

        m_custom_threads.emplace_back(new std::thread([client_desc]()
                                                      {
            custom_tester_client client(client_desc);
            client.run(); }));

        break;
    }
    case (server):
    {
        custom_tester_server_description server_desc;
        server_desc.server_ip = ip;
        server_desc.port = port;
        server_desc.datagram.size = custom_datagram_size;

        m_custom_threads.emplace_back(new std::thread([server_desc]()
                                                      {
            custom_tester_server server(server_desc);
            server.run(); }));

        break;
    }
    }
}