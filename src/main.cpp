#include <iostream>
#include <unistd.h>
#include "iperf.h"
#include "stress.h"

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 2) {
        std::cout << "[main][#1] Please enter a duration." << std::endl;
    }
    int duration = std::stoi(argv[1]);

    system("killall iperf3");
    iperf my_iperf(duration);
    my_iperf.server_start();

    sleep(1);
    std::cout << "[Waiting for server...] Please hit enter when all machines are in this state." << std::endl;
    std::cin.get();

    std::cout << std::endl << "[main] Starting RealStress..." << std::endl << std::endl;

    // CPU
    stress my_cpu_stress_100(CPU_RT, 4, duration);
    stress my_cpu_stress_5  (CPU_RT, 20, duration, 5);

    // IO
    stress my_io_stress     (IO, 8, duration);

    // Timer
    stress my_timer_stress  (TIMER, 1, duration);

    // Network
    my_iperf.client_start();

    sleep(duration + 10);
    return 0;
}