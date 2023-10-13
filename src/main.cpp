#include <iostream>
#include <unistd.h>
#include "iperf.h"
#include "stress.h"

using namespace std;

int main(int argc, char **argv)
{
    int duration = 0;
    std::string location = "hpc";
    if(argc < 2) {
        std::cout << "[main][#1] Please enter a duration." << std::endl;
    }
    else if (argc == 2) {
        duration = std::stoi(argv[1]);
        std::cout << "[main][#2] No location entered. Defaulting to \"hpc\"." << std::endl << std::endl;
    }
    else if (argc == 3) {
        duration = std::stoi(argv[1]);
        std::string location = std::string(argv[2]);
    }

    system("killall iperf3");
    iperf my_iperf(duration, location);
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
