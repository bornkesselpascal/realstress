#include "custom_tester.h"
#include "helpers.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <unistd.h>

const long long int loss_limit = 50;

//                    _ _         _
//                 __| (_)___ _ _| |_
//                / _| | / -_) ' \  _|
//  custom_tester_\__|_|_\___|_||_\__|
//

custom_tester_client::custom_tester_client(custom_tester_client_description description)
    : m_description(description), m_comm_client(m_description.server_ip, m_description.port, false)
{
}

void custom_tester_client::run()
{
    void *data = malloc(m_description.datagram.size);
    communication::udp::message_type *msg_type = (communication::udp::message_type *)data;
    *msg_type = communication::udp::CDATA_MSG;

    long long int msg_counter = 0;
    int tmr_misses = 0;

    struct timespec start_time, end_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    end_time.tv_sec = start_time.tv_sec + m_description.duration;
    end_time.tv_nsec = start_time.tv_nsec;

    helpers::timer gap_timer;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // FIXED DATAGRAM SIZE (size)
    size_t msg_size = m_description.datagram.size;
    gap_timer.initialize(m_description.gap);

    while (true)
    {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (current_time.tv_sec >= end_time.tv_sec)
        {
            break;
        }

        m_comm_client.send(data, msg_size);
        msg_counter++;

        tmr_misses += gap_timer.wait();
    }

    free(data);
    sleep(4);

    communication::udp::message_type end_message = communication::udp::CSTOP_MSG;
    int bytes_send = m_comm_client.send(&end_message, sizeof(end_message));
    if (bytes_send == -1)
    {
        std::cerr << "[custom_tester] Error when sending end message." << std::endl;
    }

    close(m_comm_client.get_socket());
}

//
//                 ___ ___ _ ___ _____ _ _
//                (_-</ -_) '_\ V / -_) '_|
//  custom_tester_/__/\___|_|  \_/\___|_|
//

custom_tester_server::custom_tester_server(custom_tester_server_description description)
    : m_description(description), m_comm_server(m_description.server_ip, m_description.port)
{
}

void custom_tester_server::run()
{
    void *data = malloc(m_description.datagram.size);
    communication::udp::message_type *msg_type = (communication::udp::message_type *)data;

    long long int msg_counter = 0;

    while (1)
    {
        m_comm_server.receive(data, m_description.datagram.size);

        if (*msg_type == communication::udp::message_type::CSTOP_MSG)
        {
            break;
        }
        msg_counter++;
    }

    free(data);
    close(m_comm_client.get_socket());
}
