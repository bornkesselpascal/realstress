#ifndef IPERF_H
#define IPERF_H

#include <string>

enum process_type {
    client,
    server,
};

enum target_location {
    hpc,
    tpc,
};

class network
{
public:
    network(int duration, target_location location, std::string bandwidth_limit = "1G");
    ~network();
    void server_start();
    void client_start();

private:
    bool create_iperf_process(process_type type, int port, std::string ip = "");

    enum pc_type {
        hpc1,
        hpc2,
        tpc1,
        tpc2,
    } m_pc_type;
    target_location m_target_location;
    std::string m_bandwidth_limit;
    int m_duration;
};

#endif // IPERF_H
