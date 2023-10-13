#ifndef IPERF_H
#define IPERF_H

#include <string>

enum process_type {
    client,
    server,
};

class iperf
{
public:
    iperf(int duration, std::string location, std::string bandwidth_limit = "1G");
    ~iperf();
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
    enum target_location {
    hpc,
    tpc,
    } m_target_location;
    std::string m_bandwidth_limit;
    int m_duration;
};

#endif // IPERF_H
