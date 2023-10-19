#ifndef IPERF_H
#define IPERF_H

#include <string>
#include <vector>

enum process_type {
    client,
    server,
};

enum target_location {
    hpc,
    tpc,
};

enum network_method {
    iperf,
    custom,
};

class network
{
public:
    network(int duration, target_location location, network_method method,  std::string bandwidth_limit = "1G");
    ~network();
    void server_start();
    void client_start();

private:
    bool create_network_process(process_type type, int port, std::string ip = "");

    enum pc_type {
        hpc1,
        hpc2,
        tpc1,
        tpc2,
    } m_pc_type;
    target_location m_target_location;
    network_method m_method;
    std::string m_bandwidth_limit;
    int m_duration;
    std::vector<std::unique_ptr<std::thread>> m_custom_threads;
};

#endif // IPERF_H
